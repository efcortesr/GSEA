#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <sys/types.h>
#include <limits.h>

// Ensure PATH_MAX is defined
#ifndef PATH_MAX
#ifdef __linux__
#include <linux/limits.h>
#endif
#endif
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

// Single-file RLE (no table)
int compress_file_rle(const char *src, const char *dest);
int decompress_file_rle(const char *src, const char *dest);

// Directory RLE (no table)
int compress_directory_rle(const char *src_dir, const char *dest_dir);
int decompress_directory_rle(const char *src_dir, const char *dest_dir);

// ===== Reporting structs and API =====
typedef struct
{
    char name[PATH_MAX];
    off_t input_size;
    off_t output_size;
    double elapsed_ms; // time per file
    int rc;            // 0 OK, !=0 error
} FMResult;

// Single-file with table report
int compress_file_rle_with_report(const char *src, const char *dest);
int decompress_file_rle_with_report(const char *src, const char *dest);

// Directory (concurrent) with consolidated table report
int compress_directory_rle_with_report(const char *src_dir, const char *dest_dir);
int decompress_directory_rle_with_report(const char *src_dir, const char *dest_dir);

#endif // FILE_MANAGER_H
