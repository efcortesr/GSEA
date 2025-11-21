#define _POSIX_C_SOURCE 200809L
#include "encryptor.h"

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/limits.h>
#include "file_manager.h"
#include <time.h>

/* ===========================================================
 *                HELPERS COMUNES
 * =========================================================== */

static int write_all(int fd, const uint8_t *buf, size_t n)
{
    size_t off = 0;
    while (off < n)
    {
        ssize_t w = write(fd, buf + off, n - off);
        if (w < 0)
        {
            perror("write");
            return -1;
        }
        off += (size_t)w;
    }
    return 0;
}

/* Implementación del cifrado Vigenère sobre un bloque en memoria.
 * IMPORTANTE: el índice de la clave se reinicia en cada bloque,
 * igual que en la versión secuencial actual (por cada lectura).
 */
static void vigenere_process_block(uint8_t *data,
                                   size_t len,
                                   const char *key,
                                   size_t key_len,
                                   int encrypt)
{
    for (size_t i = 0; i < len; i++)
    {
        char k = key[i % key_len];
        if (encrypt)
        {
            data[i] = (uint8_t)((data[i] + k) & 0xFF);
        }
        else
        {
            data[i] = (uint8_t)((data[i] - k) & 0xFF);
        }
    }
}

/* ===========================================================
 *       CIFRADO / DESCIFRADO SECUENCIAL (STREAM)
 * =========================================================== */

static int vigenere_process_stream(int fd_in, int fd_out,
                                   const char *key,
                                   int encrypt)
{
    if (!key || !*key)
    {
        fprintf(stderr, "Empty key not allowed\n");
        return 1;
    }

    size_t key_len = strlen(key);
    uint8_t *buf = malloc(VIGENERE_BLOCK_SIZE);
    if (!buf)
    {
        perror("malloc");
        return 1;
    }

    for (;;)
    {
        ssize_t n = read(fd_in, buf, VIGENERE_BLOCK_SIZE);
        if (n < 0)
        {
            perror("read");
            free(buf);
            return 2;
        }
        if (n == 0)
        {
            break;
        }

        vigenere_process_block(buf, (size_t)n, key, key_len, encrypt);

        if (write_all(fd_out, buf, (size_t)n) != 0)
        {
            free(buf);
            return 3;
        }
    }

    free(buf);
    return 0;
}

int vigenere_encrypt_stream(int fd_in, int fd_out, const char *key)
{
    return vigenere_process_stream(fd_in, fd_out, key, 1);
}

int vigenere_decrypt_stream(int fd_in, int fd_out, const char *key)
{
    return vigenere_process_stream(fd_in, fd_out, key, 0);
}

/* ===========================================================
 *     CIFRADO / DESCIFRADO PARA UN SOLO ARCHIVO (PARALELO)
 * =========================================================== */

/* Umbral a partir del cual usamos varios hilos para un archivo.
 * Puedes ajustarlo (por ejemplo 8 MiB).
 */
#define PARALLEL_FILE_THRESHOLD (1 * 1024 * 1024)

typedef struct
{
    int fd_in;
    int fd_out;
    off_t offset;  /* inicio de este bloque dentro del archivo */
    size_t length; /* longitud de este bloque */
    const char *key;
    size_t key_len;
    int encrypt;   /* 1 = cifrar, 0 = descifrar */
    int thread_id; /* para logs */
    int rc;        /* resultado del hilo */
} FileBlockTask;

/* Hilo para procesar un bloque concreto de un archivo con pread/pwrite */
static void *thread_vigenere_block(void *arg)
{
    FileBlockTask *t = (FileBlockTask *)arg;

    uint8_t *buf = malloc(VIGENERE_BLOCK_SIZE);
    if (!buf)
    {
        perror("malloc");
        t->rc = 1;
        return NULL;
    }

    off_t pos = t->offset;
    size_t remaining = t->length;

    printf("[FileThread %d] Processing offset %lld, length %zu bytes\n",
           t->thread_id, (long long)t->offset, t->length);

    while (remaining > 0)
    {
        size_t to_read = remaining > VIGENERE_BLOCK_SIZE ? VIGENERE_BLOCK_SIZE : remaining;

        ssize_t r = pread(t->fd_in, buf, to_read, pos);
        if (r < 0)
        {
            perror("pread");
            t->rc = 2;
            break;
        }
        if (r == 0)
        {
            /* EOF inesperado */
            fprintf(stderr, "[FileThread %d] Unexpected EOF\n", t->thread_id);
            t->rc = 3;
            break;
        }

        vigenere_process_block(buf, (size_t)r, t->key, t->key_len, t->encrypt);

        ssize_t w = pwrite(t->fd_out, buf, (size_t)r, pos);
        if (w < 0)
        {
            perror("pwrite");
            t->rc = 4;
            break;
        }
        if (w != r)
        {
            fprintf(stderr, "[FileThread %d] Short write\n", t->thread_id);
            t->rc = 5;
            break;
        }

        remaining -= (size_t)r;
        pos += (off_t)r;
    }

    if (t->rc == 0)
    {
        printf("[FileThread %d] Done block offset %lld\n",
               t->thread_id, (long long)t->offset);
    }

    free(buf);
    return NULL;
}

/* Procesa un archivo completo.
 * Si es grande y hay varios CPUs, se divide en bloques
 * y se usa un hilo por bloque. Si no, se usa la versión secuencial.
 */
static int vigenere_file_parallel(const char *src,
                                  const char *dest,
                                  const char *key,
                                  int encrypt)
{
    if (!key || !*key)
    {
        fprintf(stderr, "Empty key not allowed\n");
        return 1;
    }

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

    struct stat st;
    if (fstat(fd_in, &st) != 0)
    {
        perror("fstat");
        close(fd_in);
        close(fd_out);
        return 1;
    }

    off_t filesize = st.st_size;

    /* Archivos muy pequeños: mejor secuencial para evitar overhead */
    long nproc = sysconf(_SC_NPROCESSORS_ONLN);
    if (nproc < 1)
        nproc = 1;
    if (nproc > MAX_CRYPTO_THREADS)
        nproc = MAX_CRYPTO_THREADS;

    if (filesize == 0 || filesize < PARALLEL_FILE_THRESHOLD || nproc == 1)
    {
        int rc = vigenere_process_stream(fd_in, fd_out, key, encrypt);
        close(fd_in);
        close(fd_out);
        return rc;
    }

    /* Paralelo por bloques */
    if (ftruncate(fd_out, filesize) != 0)
    {
        perror("ftruncate");
        close(fd_in);
        close(fd_out);
        return 1;
    }

    size_t key_len = strlen(key);

    long nthreads = nproc;
    off_t chunk = (filesize + nthreads - 1) / nthreads;

    pthread_t *threads = calloc((size_t)nthreads, sizeof(pthread_t));
    FileBlockTask *tasks = calloc((size_t)nthreads, sizeof(FileBlockTask));
    if (!threads || !tasks)
    {
        perror("calloc");
        free(threads);
        free(tasks);
        close(fd_in);
        close(fd_out);
        return 1;
    }

    int tcount = 0;
    for (long i = 0; i < nthreads; i++)
    {
        off_t offset = (off_t)i * chunk;
        if (offset >= filesize)
            break;

        off_t end = offset + chunk;
        if (end > filesize)
            end = filesize;

        tasks[tcount].fd_in = fd_in;
        tasks[tcount].fd_out = fd_out;
        tasks[tcount].offset = offset;
        tasks[tcount].length = (size_t)(end - offset);
        tasks[tcount].key = key;
        tasks[tcount].key_len = key_len;
        tasks[tcount].encrypt = encrypt;
        tasks[tcount].thread_id = tcount + 1;
        tasks[tcount].rc = 0;

        if (pthread_create(&threads[tcount], NULL,
                           thread_vigenere_block, &tasks[tcount]) != 0)
        {
            perror("pthread_create");
            break;
        }
        tcount++;
    }

    int final_rc = 0;
    for (int i = 0; i < tcount; i++)
    {
        pthread_join(threads[i], NULL);
        if (tasks[i].rc != 0 && final_rc == 0)
            final_rc = tasks[i].rc;
    }

    free(threads);
    free(tasks);
    close(fd_in);
    close(fd_out);

    return final_rc;
}

int encrypt_file(const char *src, const char *dest, const char *key)
{
    return vigenere_file_parallel(src, dest, key, 1);
}

int decrypt_file(const char *src, const char *dest, const char *key)
{
    return vigenere_file_parallel(src, dest, key, 0);
}

/* ===========================================================
 *      CIFRADO / DESCIFRADO DE DIRECTORIOS (UN HILO POR ARCHIVO)
 * =========================================================== */

static void *thread_encrypt(void *arg)
{
    crypto_thread_data_t *data = (crypto_thread_data_t *)arg;
    printf("[DirThread %d] Encrypting: %s -> %s\n",
           data->thread_id, data->src, data->dest);
    data->result = encrypt_file(data->src, data->dest, data->key);
    if (data->result == 0)
        printf("[DirThread %d] Done encrypting %s\n", data->thread_id, data->src);
    else
        fprintf(stderr, "[DirThread %d] Error encrypting %s\n",
                data->thread_id, data->src);
    return NULL;
}

static void *thread_decrypt(void *arg)
{
    crypto_thread_data_t *data = (crypto_thread_data_t *)arg;
    printf("[DirThread %d] Decrypting: %s -> %s\n",
           data->thread_id, data->src, data->dest);
    data->result = decrypt_file(data->src, data->dest, data->key);
    if (data->result == 0)
        printf("[DirThread %d] Done decrypting %s\n", data->thread_id, data->src);
    else
        fprintf(stderr, "[DirThread %d] Error decrypting %s\n",
                data->thread_id, data->src);
    return NULL;
}

static int process_directory(const char *src_dir,
                             const char *dest_dir,
                             const char *key,
                             int encrypt)
{
    DIR *dir;
    struct dirent *entry;
    pthread_t threads[MAX_CRYPTO_THREADS];
    crypto_thread_data_t thread_data[MAX_CRYPTO_THREADS];
    int thread_count = 0;
    int return_code = 0;
    int next_thread_id = 1;

    mkdir(dest_dir, 0755);

    dir = opendir(src_dir);
    if (!dir)
    {
        perror("opendir");
        return 1;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_name[0] == '.')
        {
            continue;
        }

        char src_path[PATH_MAX];
        char dest_path[PATH_MAX];
        snprintf(src_path, sizeof(src_path), "%s/%s", src_dir, entry->d_name);
        snprintf(dest_path, sizeof(dest_path), "%s/%s", dest_dir, entry->d_name);

        struct stat st;
        if (stat(src_path, &st) != 0)
        {
            perror("stat");
            continue;
        }

        if (!S_ISREG(st.st_mode))
        {
            continue;
        }

        char *src_copy = strdup(src_path);
        char *dest_copy = strdup(dest_path);
        if (!src_copy || !dest_copy)
        {
            perror("strdup");
            free(src_copy);
            free(dest_copy);
            return 1;
        }

        thread_data[thread_count].src = src_copy;
        thread_data[thread_count].dest = dest_copy;
        thread_data[thread_count].key = key;
        thread_data[thread_count].result = 0;
        thread_data[thread_count].thread_id = next_thread_id++;

        pthread_create(&threads[thread_count],
                       NULL,
                       encrypt ? thread_encrypt : thread_decrypt,
                       &thread_data[thread_count]);

        thread_count++;

        if (thread_count == MAX_CRYPTO_THREADS)
        {
            for (int i = 0; i < thread_count; i++)
            {
                pthread_join(threads[i], NULL);
                if (thread_data[i].result != 0)
                {
                    return_code = thread_data[i].result;
                }
                free((char *)thread_data[i].src);
                free((char *)thread_data[i].dest);
            }
            thread_count = 0;
        }
    }

    for (int i = 0; i < thread_count; i++)
    {
        pthread_join(threads[i], NULL);
        if (thread_data[i].result != 0)
        {
            return_code = thread_data[i].result;
        }
        free((char *)thread_data[i].src);
        free((char *)thread_data[i].dest);
    }

    closedir(dir);
    return return_code;
}

int encrypt_directory(const char *src_dir, const char *dest_dir, const char *key)
{
    return process_directory(src_dir, dest_dir, key, 1);
}

int decrypt_directory(const char *src_dir, const char *dest_dir, const char *key)
{
    return process_directory(src_dir, dest_dir, key, 0);
}

/* ============== REPORTING WRAPPERS FOR ENCRYPT/DECRYPT ============== */

static inline long long now_ns_local(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000000000LL + (long long)ts.tv_nsec;
}

static inline double ns_to_ms_local(long long ns)
{
    return (double)ns / 1.0e6;
}

static void print_time_table(const char *title, const FMResult *rows, int nrows)
{
    printf("\n===== %s =====\n", title);
    printf("%-40s  %12s  %s\n", "File", "Time (ms)", "Status");
    printf("%-40s  %12s  %s\n", "----------------------------------------", "----------", "------");
    double sum_ms = 0.0;
    for (int i = 0; i < nrows; i++)
    {
        printf("%-40.40s  %12.2f  %s\n",
               rows[i].name, rows[i].elapsed_ms,
               (rows[i].rc == 0 ? "OK" : "ERR"));
        sum_ms += rows[i].elapsed_ms;
    }
    printf("%-40s  %12.2f  %s\n", "[TOTAL]", sum_ms, "-");
    printf("==============================================\n\n");
}

int encrypt_file_with_report(const char *src, const char *dest, const char *key)
{
    FMResult row;
    memset(&row, 0, sizeof row);

    const char *slash = strrchr(src, '/');
    snprintf(row.name, sizeof(row.name), "%s", slash ? slash + 1 : src);

    long long t0 = now_ns_local();
    int rc = encrypt_file(src, dest, key);
    long long t1 = now_ns_local();

    row.rc = rc;
    row.elapsed_ms = ns_to_ms_local(t1 - t0);

    print_time_table("Encryption Report", &row, 1);
    return rc;
}

int decrypt_file_with_report(const char *src, const char *dest, const char *key)
{
    FMResult row;
    memset(&row, 0, sizeof row);

    const char *slash = strrchr(src, '/');
    snprintf(row.name, sizeof(row.name), "%s", slash ? slash + 1 : src);

    long long t0 = now_ns_local();
    int rc = decrypt_file(src, dest, key);
    long long t1 = now_ns_local();

    row.rc = rc;
    row.elapsed_ms = ns_to_ms_local(t1 - t0);

    print_time_table("Decryption Report", &row, 1);
    return rc;
}

int encrypt_directory_with_report(const char *src_dir, const char *dest_dir, const char *key)
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

    const int MAX_FILES = 8192;
    FMResult *results = calloc(MAX_FILES, sizeof(FMResult));
    if (!results)
    {
        perror("calloc");
        closedir(dir);
        return 1;
    }
    int results_count = 0;

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

        snprintf(output_path, sizeof(output_path), "%s/%s", dest_dir, entry->d_name);

        if (results_count >= MAX_FILES)
        {
            fprintf(stderr, "Too many files, increase MAX_FILES\n");
            break;
        }

        FMResult *row = &results[results_count++];
        snprintf(row->name, sizeof(row->name), "%s", entry->d_name);

        long long t0 = now_ns_local();
        int rc = encrypt_file(input_path, output_path, key);
        long long t1 = now_ns_local();

        row->rc = rc;
        row->elapsed_ms = ns_to_ms_local(t1 - t0);
    }

    closedir(dir);

    print_time_table("Encryption Directory Report", results, results_count);
    free(results);
    return 0;
}

int decrypt_directory_with_report(const char *src_dir, const char *dest_dir, const char *key)
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

    const int MAX_FILES = 8192;
    FMResult *results = calloc(MAX_FILES, sizeof(FMResult));
    if (!results)
    {
        perror("calloc");
        closedir(dir);
        return 1;
    }
    int results_count = 0;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        size_t len = strlen(entry->d_name);
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

        snprintf(output_path, sizeof(output_path), "%s/%s", dest_dir, entry->d_name);

        if (results_count >= MAX_FILES)
        {
            fprintf(stderr, "Too many files, increase MAX_FILES\n");
            break;
        }

        FMResult *row = &results[results_count++];
        snprintf(row->name, sizeof(row->name), "%s", entry->d_name);

        long long t0 = now_ns_local();
        int rc = decrypt_file(input_path, output_path, key);
        long long t1 = now_ns_local();

        row->rc = rc;
        row->elapsed_ms = ns_to_ms_local(t1 - t0);
    }

    closedir(dir);

    print_time_table("Decryption Directory Report", results, results_count);
    free(results);
    return 0;
}
