#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pthread.h>
#include <limits.h>
#include <time.h>

#ifndef PATH_MAX
#ifdef __linux__
#include <linux/limits.h>
#endif
#endif
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#include "compressor.h"
#include "file_manager.h"

/* ===========================================================
 *                  TIME / SIZE / TABLE HELPERS
 * =========================================================== */

static inline long long now_ns(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000000000LL + (long long)ts.tv_nsec;
}

static inline double ns_to_ms(long long ns)
{
    return (double)ns / 1.0e6;
}

static off_t get_file_size_or_minus1(const char *path)
{
    struct stat st;
    if (stat(path, &st) == -1)
        return (off_t)-1;
    return st.st_size;
}

static void human_bytes(off_t n, char *out, size_t outsz)
{
    const char *units[] = {"B", "KB", "MB", "GB", "TB"};
    double v = (double)((n < 0) ? 0 : n);
    int u = 0;
    while (v >= 1024.0 && u < 4)
    {
        v /= 1024.0;
        u++;
    }
    snprintf(out, outsz, (u == 0) ? "%.0f %s" : "%.2f %s", v, units[u]);
}

static void print_results_table(const char *title, const FMResult *rows, int nrows)
{
    printf("\n===== %s =====\n", title);
    printf("%-40s  %14s  %14s  %12s  %10s  %s\n",
           "File", "Input Size", "Output Size", "Delta", "Time (ms)", "Status");
    printf("%-40s  %14s  %14s  %12s  %10s  %s\n",
           "----------------------------------------",
           "--------------", "--------------", "------------", "----------", "------");

    off_t sum_in = 0, sum_out = 0;
    double sum_ms = 0.0;

    for (int i = 0; i < nrows; i++)
    {
        char in_h[32], out_h[32], delta_h[32];
        off_t delta = rows[i].output_size - rows[i].input_size;
        human_bytes(rows[i].input_size, in_h, sizeof in_h);
        human_bytes(rows[i].output_size, out_h, sizeof out_h);
        human_bytes(delta, delta_h, sizeof delta_h);

        printf("%-40.40s  %14s  %14s  %12s  %10.2f  %s\n",
               rows[i].name, in_h, out_h, delta_h, rows[i].elapsed_ms,
               (rows[i].rc == 0 ? "OK" : "ERR"));

        if (rows[i].input_size >= 0)
            sum_in += rows[i].input_size;
        if (rows[i].output_size >= 0)
            sum_out += rows[i].output_size;
        sum_ms += rows[i].elapsed_ms;
    }

    char sum_in_h[32], sum_out_h[32], sum_delta_h[32];
    human_bytes(sum_in, sum_in_h, sizeof sum_in_h);
    human_bytes(sum_out, sum_out_h, sizeof sum_out_h);
    human_bytes(sum_out - sum_in, sum_delta_h, sizeof sum_delta_h);

    printf("%-40s  %14s  %14s  %12s  %10.2f  %s\n",
           "[TOTAL]", sum_in_h, sum_out_h, sum_delta_h, sum_ms, "-");
    printf("==============================================\n\n");
}

/* ===========================================================
 *               BASIC FILE OPERATIONS (SERIAL)
 * =========================================================== */

int compress_file_rle(const char *src, const char *dest)
{
    int fd_in = open(src, O_RDONLY);
    if (fd_in < 0)
    {
        perror("open input");
        return 1;
    }

    int fd_out = open(dest, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_out < 0)
    {
        perror("open output");
        close(fd_in);
        return 1;
    }

    int rc = rle2_compress_stream(fd_in, fd_out);

    close(fd_in);
    close(fd_out);
    return rc;
}

int decompress_file_rle(const char *src, const char *dest)
{
    int fd_in = open(src, O_RDONLY);
    if (fd_in < 0)
    {
        perror("open input");
        return 1;
    }

    int fd_out = open(dest, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_out < 0)
    {
        perror("open output");
        close(fd_in);
        return 1;
    }

    uint8_t hdr[8];
    ssize_t r = read(fd_in, hdr, sizeof(hdr));
    if (r < 0)
    {
        perror("read");
        close(fd_in);
        close(fd_out);
        return 1;
    }
    if (r != 8)
    {
        fprintf(stderr, "Invalid header (too short).\n");
        close(fd_in);
        close(fd_out);
        return 1;
    }

    if (lseek(fd_in, 0, SEEK_SET) < 0)
    {
        perror("lseek");
        close(fd_in);
        close(fd_out);
        return 1;
    }

    int rc;
    if (memcmp(hdr, "RLE2\0\0\0\0", 8) == 0)
    {
        rc = rle2_decompress_stream(fd_in, fd_out);
    }
    else
    {
        fprintf(stderr, "Unknown format (not RLE2).\n");
        rc = 1;
    }

    close(fd_in);
    close(fd_out);
    return rc;
}

/* ===========================================================
 *                  CONCURRENT COMPRESSION
 * =========================================================== */

typedef struct
{
    char input_path[PATH_MAX];
    char output_path[PATH_MAX];

    int thread_id; /* ID del hilo para logs */

    int index; /* posicion en el array results[] */
    FMResult *results;
} ThreadTask;

static void *thread_compress_rle(void *arg)
{
    ThreadTask *task = (ThreadTask *)arg;

    FMResult *row = NULL;
    if (task->results && task->index >= 0)
    {
        row = &task->results[task->index];
        const char *slash = strrchr(task->input_path, '/');
        snprintf(row->name, sizeof(row->name), "%s",
                 slash ? slash + 1 : task->input_path);
        row->input_size = get_file_size_or_minus1(task->input_path);
    }

    printf("[CompThread %d] Compressing: %s -> %s\n",
           task->thread_id, task->input_path, task->output_path);
    long long t0 = now_ns();
    int rc = compress_file_rle(task->input_path, task->output_path);
    long long t1 = now_ns();

    if (row)
    {
        row->rc = rc;
        row->elapsed_ms = ns_to_ms(t1 - t0);
        row->output_size = get_file_size_or_minus1(task->output_path);
    }

    if (rc != 0)
        fprintf(stderr, "[CompThread %d] Error compressing %s\n",
                task->thread_id, task->input_path);
    else
        printf("[CompThread %d] Done: %s\n",
               task->thread_id, task->input_path);

    free(task);
    return NULL;
}

int compress_directory_rle(const char *src_dir, const char *dest_dir)
{
    DIR *dir = opendir(src_dir);
    if (!dir)
    {
        perror("opendir");
        return 1;
    }

    struct stat st_out;
    if (stat(dest_dir, &st_out) == -1)
    {
        if (mkdir(dest_dir, 0755) == -1 && errno != EEXIST)
        {
            perror("mkdir dest_dir");
            closedir(dir);
            return 1;
        }
    }

    const int MAX_THREADS = 8;
    pthread_t threads[MAX_THREADS];
    int thread_count = 0;
    int next_thread_id = 1;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char input_path[PATH_MAX];
        char output_path[PATH_MAX];

        snprintf(input_path, sizeof(input_path), "%s/%s", src_dir, entry->d_name);

        struct stat st;
        if (stat(input_path, &st) == -1)
        {
            perror("stat");
            continue;
        }
        if (!S_ISREG(st.st_mode))
            continue;

        snprintf(output_path, sizeof(output_path), "%s/%s.rle",
                 dest_dir, entry->d_name);

        ThreadTask *task = (ThreadTask *)malloc(sizeof(ThreadTask));
        if (!task)
        {
            fprintf(stderr, "malloc failed\n");
            continue;
        }
        memset(task, 0, sizeof(*task));

        strncpy(task->input_path, input_path, sizeof(task->input_path) - 1);
        task->input_path[sizeof(task->input_path) - 1] = '\0';
        strncpy(task->output_path, output_path, sizeof(task->output_path) - 1);
        task->output_path[sizeof(task->output_path) - 1] = '\0';

        task->index = -1;
        task->results = NULL;
        task->thread_id = next_thread_id++;

        if (pthread_create(&threads[thread_count], NULL,
                           thread_compress_rle, task) != 0)
        {
            perror("pthread_create");
            free(task);
            continue;
        }

        thread_count++;

        if (thread_count >= MAX_THREADS)
        {
            for (int i = 0; i < thread_count; i++)
                pthread_join(threads[i], NULL);
            thread_count = 0;
        }
    }

    closedir(dir);

    for (int i = 0; i < thread_count; i++)
        pthread_join(threads[i], NULL);

    printf("All compression threads completed.\n");
    return 0;
}

/* ===========================================================
 *                  CONCURRENT DECOMPRESSION
 * =========================================================== */

static void make_output_name_from_rle(const char *name,
                                      char *out,
                                      size_t outsz)
{
    size_t len = strlen(name);
    if (len >= 4 && strcmp(name + len - 4, ".rle") == 0)
    {
        size_t base = len - 4;
        if (base >= outsz)
            base = outsz - 1;
        memcpy(out, name, base);
        out[base] = '\0';
    }
    else
    {
        snprintf(out, outsz, "%s.out", name);
    }
}

static void *thread_decompress_rle(void *arg)
{
    ThreadTask *task = (ThreadTask *)arg;

    FMResult *row = NULL;
    if (task->results && task->index >= 0)
    {
        row = &task->results[task->index];
        const char *slash = strrchr(task->input_path, '/');
        snprintf(row->name, sizeof(row->name), "%s",
                 slash ? slash + 1 : task->input_path);
        row->input_size = get_file_size_or_minus1(task->input_path);
    }

    printf("[DecompThread %d] Decompressing: %s -> %s\n",
           task->thread_id, task->input_path, task->output_path);
    long long t0 = now_ns();
    int rc = decompress_file_rle(task->input_path, task->output_path);
    long long t1 = now_ns();

    if (row)
    {
        row->rc = rc;
        row->elapsed_ms = ns_to_ms(t1 - t0);
        row->output_size = get_file_size_or_minus1(task->output_path);
    }

    if (rc != 0)
        fprintf(stderr, "[DecompThread %d] Error decompressing %s\n",
                task->thread_id, task->input_path);
    else
        printf("[DecompThread %d] Done: %s\n",
               task->thread_id, task->input_path);

    free(task);
    return NULL;
}

int decompress_directory_rle(const char *src_dir, const char *dest_dir)
{
    DIR *dir = opendir(src_dir);
    if (!dir)
    {
        perror("opendir");
        return 1;
    }

    struct stat st_out;
    if (stat(dest_dir, &st_out) == -1)
    {
        if (mkdir(dest_dir, 0755) == -1 && errno != EEXIST)
        {
            perror("mkdir dest_dir");
            closedir(dir);
            return 1;
        }
    }

    const int MAX_THREADS = 64;
    pthread_t threads[MAX_THREADS];
    int thread_count = 0;
    int next_thread_id = 1;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        size_t len = strlen(entry->d_name);
        if (!(len >= 4 && strcmp(entry->d_name + len - 4, ".rle") == 0))
            continue;

        char input_path[PATH_MAX];
        snprintf(input_path, sizeof(input_path), "%s/%s",
                 src_dir, entry->d_name);

        struct stat st;
        if (stat(input_path, &st) == -1)
        {
            perror("stat");
            continue;
        }
        if (!S_ISREG(st.st_mode))
            continue;

        char base_out[PATH_MAX];
        make_output_name_from_rle(entry->d_name, base_out, sizeof(base_out));

        char output_path[PATH_MAX];
        snprintf(output_path, sizeof(output_path), "%s/%s",
                 dest_dir, base_out);

        ThreadTask *task = (ThreadTask *)malloc(sizeof(ThreadTask));
        if (!task)
        {
            fprintf(stderr, "malloc failed\n");
            continue;
        }
        memset(task, 0, sizeof(*task));

        strncpy(task->input_path, input_path, sizeof(task->input_path) - 1);
        task->input_path[sizeof(task->input_path) - 1] = '\0';
        strncpy(task->output_path, output_path, sizeof(task->output_path) - 1);
        task->output_path[sizeof(task->output_path) - 1] = '\0';

        task->index = -1;
        task->results = NULL;
        task->thread_id = next_thread_id++;

        if (pthread_create(&threads[thread_count], NULL,
                           thread_decompress_rle, task) != 0)
        {
            perror("pthread_create");
            free(task);
            continue;
        }

        thread_count++;

        if (thread_count >= MAX_THREADS)
        {
            for (int i = 0; i < thread_count; i++)
                pthread_join(threads[i], NULL);
            thread_count = 0;
        }
    }

    closedir(dir);

    for (int i = 0; i < thread_count; i++)
        pthread_join(threads[i], NULL);

    printf("All decompression threads completed.\n");
    return 0;
}

/* ===========================================================
 *             WITH-REPORT VARIANTS (TABLE OUTPUT)
 * =========================================================== */

int compress_file_rle_with_report(const char *src, const char *dest)
{
    FMResult row;
    memset(&row, 0, sizeof row);

    const char *slash = strrchr(src, '/');
    snprintf(row.name, sizeof(row.name), "%s", slash ? slash + 1 : src);

    row.input_size = get_file_size_or_minus1(src);

    long long t0 = now_ns();
    int rc = compress_file_rle(src, dest);
    long long t1 = now_ns();

    row.rc = rc;
    row.elapsed_ms = ns_to_ms(t1 - t0);
    row.output_size = get_file_size_or_minus1(dest);

    print_results_table("Compression Report", &row, 1);
    return rc;
}

int decompress_file_rle_with_report(const char *src, const char *dest)
{
    FMResult row;
    memset(&row, 0, sizeof row);

    const char *slash = strrchr(src, '/');
    snprintf(row.name, sizeof(row.name), "%s", slash ? slash + 1 : src);

    row.input_size = get_file_size_or_minus1(src);

    long long t0 = now_ns();
    int rc = decompress_file_rle(src, dest);
    long long t1 = now_ns();

    row.rc = rc;
    row.elapsed_ms = ns_to_ms(t1 - t0);
    row.output_size = get_file_size_or_minus1(dest);

    print_results_table("Decompression Report", &row, 1);
    return rc;
}

int compress_directory_rle_with_report(const char *src_dir, const char *dest_dir)
{
    DIR *dir = opendir(src_dir);
    if (!dir)
    {
        perror("opendir");
        return 1;
    }

    struct stat st_out;
    if (stat(dest_dir, &st_out) == -1)
    {
        if (mkdir(dest_dir, 0755) == -1 && errno != EEXIST)
        {
            perror("mkdir dest_dir");
            closedir(dir);
            return 1;
        }
    }

    const int MAX_THREADS = 64;
    const int MAX_FILES = 8192;
    pthread_t threads[MAX_THREADS];
    int thread_count = 0;
    int next_thread_id = 1;

    FMResult *results = (FMResult *)calloc(MAX_FILES, sizeof(FMResult));
    if (!results)
    {
        perror("calloc");
        closedir(dir);
        return 1;
    }
    int results_count = 0;

    long long T0 = now_ns();

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char input_path[PATH_MAX];
        snprintf(input_path, sizeof(input_path), "%s/%s",
                 src_dir, entry->d_name);

        struct stat st;
        if (stat(input_path, &st) == -1)
        {
            perror("stat");
            continue;
        }
        if (!S_ISREG(st.st_mode))
            continue;

        char output_path[PATH_MAX];
        snprintf(output_path, sizeof(output_path), "%s/%s.rle",
                 dest_dir, entry->d_name);

        ThreadTask *task = (ThreadTask *)malloc(sizeof(ThreadTask));
        if (!task)
        {
            fprintf(stderr, "malloc failed\n");
            continue;
        }
        memset(task, 0, sizeof(*task));

        strncpy(task->input_path, input_path, sizeof(task->input_path) - 1);
        task->input_path[sizeof(task->input_path) - 1] = '\0';
        strncpy(task->output_path, output_path, sizeof(task->output_path) - 1);
        task->output_path[sizeof(task->output_path) - 1] = '\0';

        if (results_count >= MAX_FILES)
        {
            fprintf(stderr, "Too many files, increase MAX_FILES\n");
            free(task);
            break;
        }
        task->index = results_count++;
        task->results = results;
        task->thread_id = next_thread_id++;

        if (pthread_create(&threads[thread_count], NULL,
                           thread_compress_rle, task) != 0)
        {
            perror("pthread_create");
            free(task);
            results_count--; /* slot unused */
            continue;
        }
        thread_count++;

        if (thread_count >= MAX_THREADS)
        {
            for (int i = 0; i < thread_count; i++)
                pthread_join(threads[i], NULL);
            thread_count = 0;
        }
    }

    closedir(dir);
    for (int i = 0; i < thread_count; i++)
        pthread_join(threads[i], NULL);

    long long T1 = now_ns();

    print_results_table("Compression Report", results, results_count);
    double wall_ms = ns_to_ms(T1 - T0);
    printf("Wall-clock total time: %.2f ms\n", wall_ms);

    free(results);
    printf("All compression threads completed.\n");
    return 0;
}

int decompress_directory_rle_with_report(const char *src_dir, const char *dest_dir)
{
    DIR *dir = opendir(src_dir);
    if (!dir)
    {
        perror("opendir");
        return 1;
    }

    struct stat st_out;
    if (stat(dest_dir, &st_out) == -1)
    {
        if (mkdir(dest_dir, 0755) == -1 && errno != EEXIST)
        {
            perror("mkdir dest_dir");
            closedir(dir);
            return 1;
        }
    }

    const int MAX_THREADS = 64;
    const int MAX_FILES = 8192;
    pthread_t threads[MAX_THREADS];
    int thread_count = 0;
    int next_thread_id = 1;

    FMResult *results = (FMResult *)calloc(MAX_FILES, sizeof(FMResult));
    if (!results)
    {
        perror("calloc");
        closedir(dir);
        return 1;
    }
    int results_count = 0;

    long long T0 = now_ns();

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        size_t len = strlen(entry->d_name);
        if (!(len >= 4 && strcmp(entry->d_name + len - 4, ".rle") == 0))
            continue;

        char input_path[PATH_MAX];
        snprintf(input_path, sizeof(input_path), "%s/%s",
                 src_dir, entry->d_name);

        struct stat st;
        if (stat(input_path, &st) == -1)
        {
            perror("stat");
            continue;
        }
        if (!S_ISREG(st.st_mode))
            continue;

        char base_out[PATH_MAX];
        make_output_name_from_rle(entry->d_name, base_out, sizeof(base_out));

        char output_path[PATH_MAX];
        snprintf(output_path, sizeof(output_path), "%s/%s",
                 dest_dir, base_out);

        ThreadTask *task = (ThreadTask *)malloc(sizeof(ThreadTask));
        if (!task)
        {
            fprintf(stderr, "malloc failed\n");
            continue;
        }
        memset(task, 0, sizeof(*task));

        strncpy(task->input_path, input_path, sizeof(task->input_path) - 1);
        task->input_path[sizeof(task->input_path) - 1] = '\0';
        strncpy(task->output_path, output_path, sizeof(task->output_path) - 1);
        task->output_path[sizeof(task->output_path) - 1] = '\0';

        if (results_count >= MAX_FILES)
        {
            fprintf(stderr, "Too many files, increase MAX_FILES\n");
            free(task);
            break;
        }
        task->index = results_count++;
        task->results = results;
        task->thread_id = next_thread_id++;

        if (pthread_create(&threads[thread_count], NULL,
                           thread_decompress_rle, task) != 0)
        {
            perror("pthread_create");
            free(task);
            results_count--;
            continue;
        }
        thread_count++;

        if (thread_count >= MAX_THREADS)
        {
            for (int i = 0; i < thread_count; i++)
                pthread_join(threads[i], NULL);
            thread_count = 0;
        }
    }

    closedir(dir);
    for (int i = 0; i < thread_count; i++)
        pthread_join(threads[i], NULL);

    long long T1 = now_ns();

    print_results_table("Decompression Report", results, results_count);
    double wall_ms = ns_to_ms(T1 - T0);
    printf("Wall-clock total time: %.2f ms\n", wall_ms);

    free(results);
    printf("All decompression threads completed.\n");
    return 0;
}
