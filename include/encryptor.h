#ifndef ENCRYPTOR_H
#define ENCRYPTOR_H

#include <stddef.h>
#include <stdint.h>

/* Encrypt/decrypt stream API (secuencial sobre file descriptors) */
int vigenere_encrypt_stream(int fd_in, int fd_out, const char *key);
int vigenere_decrypt_stream(int fd_in, int fd_out, const char *key);

/* File operations.
 * Ahora internamente usan varios hilos para archivos grandes
 * (dividiendo el archivo en bloques) y secuencial para archivos pequeños.
 */
int encrypt_file(const char *src, const char *dest, const char *key);
int decrypt_file(const char *src, const char *dest, const char *key);

/* Variants that print a simple timing report (per-file time in ms).
 * Single-file variants print a one-row table.
 * Directory variants run files sequentially and print a table with per-file times.
 */
int encrypt_file_with_report(const char *src, const char *dest, const char *key);
int decrypt_file_with_report(const char *src, const char *dest, const char *key);
int encrypt_directory_with_report(const char *src_dir, const char *dest_dir, const char *key);
int decrypt_directory_with_report(const char *src_dir, const char *dest_dir, const char *key);

/* Directory operations with concurrency (un hilo por archivo) */
int encrypt_directory(const char *src_dir, const char *dest_dir, const char *key);
int decrypt_directory(const char *src_dir, const char *dest_dir, const char *key);

/* Thread data structure for directory encryption/decryption */
typedef struct
{
    const char *src;
    const char *dest;
    const char *key;
    int result;
    int thread_id; /* identificador del hilo (para logs) */
} crypto_thread_data_t;

#define VIGENERE_BLOCK_SIZE (64 * 1024) /* 64 KiB blocks for I/O */
#define MAX_CRYPTO_THREADS 8           /* Maximum concurrent encryption/decryption threads */

#endif
