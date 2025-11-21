#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <linux/limits.h>

#include "cli.h"
#include "file_manager.h"
#include "encryptor.h"

/**
 * Revisar si hay alguna flag de operaciones (e.g., 'c', 'd', 'e', 'u')
 * en la cadena options.operation.
 */
static int has_flag(const char *ops, char f)
{
    if (!ops)
        return 0;
    return strchr(ops, f) != NULL;
}

int main(int argc, char *argv[])
{
    ProgramOptions options;

    // Parsear argumentos con el CLI
    if (!parse_arguments(argc, argv, &options))
    {
        print_help();
        return 1;
    }

    printf("Operation: %s\n", options.operation);
    printf("Input: %s\n", options.input_path);
    printf("Output: %s\n", options.output_path);
    if (options.key)
    {
        printf("Key: %s\n", options.key);
    }

    char temp_path[PATH_MAX];
    const char *current_input = options.input_path;
    const char *final_output = options.output_path;
    int needs_cleanup = 0;
    struct stat st; // Declarar st una sola vez al inicio

    // Determinar el orden de las operaciones
    int do_compress = has_flag(options.operation, 'c');
    int do_decompress = has_flag(options.operation, 'd');
    int do_encrypt = has_flag(options.operation, 'e');
    int do_decrypt = has_flag(options.operation, 'u');

    // Ordenar las operaciones y determinar rutas temporales
    if (do_decrypt && do_decompress)
    {
        // Para -ud: primero desencriptar, luego descomprimir
        snprintf(temp_path, sizeof(temp_path), "%s.tmp", options.output_path);
        needs_cleanup = 1;

        // Para archivos individuales, usar el nombre del archivo como temporal
        if (stat(current_input, &st) == 0 && !S_ISDIR(st.st_mode))
        {
            char filename[PATH_MAX];
            snprintf(filename, sizeof(filename), "%s.tmp", options.output_path);
            strncpy(temp_path, filename, sizeof(temp_path));
        }
        else
        {
            mkdir(temp_path, 0755);
        }

        // Primero desencriptar
        if (!options.key)
        {
            fprintf(stderr, "Decryption requires a key (-k option)\n");
            return 2;
        }

        if (stat(current_input, &st) == 0 && S_ISDIR(st.st_mode))
        {
            printf("\n[MODE] Directory decryption (concurrent)\n");
            printf("Source directory : %s\n", current_input);
            printf("Target directory : %s\n\n", temp_path);
            int rc = decrypt_directory_with_report(current_input, temp_path, options.key);
            if (rc != 0)
            {
                fprintf(stderr, "Directory decryption failed.\n");
                if (needs_cleanup)
                    unlink(temp_path);
                return rc;
            }
            printf("\nDirectory decryption completed successfully.\n");
        }
        else
        {
            printf("\n[MODE] Single file decryption\n");
            int rc = decrypt_file_with_report(current_input, temp_path, options.key);
            if (rc != 0)
            {
                fprintf(stderr, "Decryption failed.\n");
                if (needs_cleanup)
                    unlink(temp_path);
                return rc;
            }
            printf("\nDecryption completed successfully.\n");
        }

        // Luego descomprimir del temporal al destino final
        current_input = temp_path;
        if (stat(current_input, &st) == 0 && S_ISDIR(st.st_mode))
        {
            printf("\n[MODE] Directory decompression (concurrent)\n");
            printf("Source directory : %s\n", current_input);
            printf("Target directory : %s\n\n", final_output);
            int rc = decompress_directory_rle_with_report(current_input, final_output);
            if (rc != 0)
            {
                fprintf(stderr, "Directory decompression failed.\n");
                if (needs_cleanup)
                    unlink(temp_path);
                return rc;
            }
            printf("\nDecompression completed successfully.\n");
        }
        else
        {
            printf("\n[MODE] Single file decompression\n");
            int rc = decompress_file_rle_with_report(current_input, final_output);
            if (rc != 0)
            {
                fprintf(stderr, "Decompression failed.\n");
                if (needs_cleanup)
                    unlink(temp_path);
                return rc;
            }
            printf("\nDecompression completed successfully.\n");
        }
    }
    else if (do_compress && do_encrypt)
    {
        // Para -ce: primero comprimir, luego encriptar
        snprintf(temp_path, sizeof(temp_path), "%s.tmp", options.output_path);
        needs_cleanup = 1;

        // Para archivos individuales, usar el nombre del archivo como temporal
        if (stat(current_input, &st) == 0 && !S_ISDIR(st.st_mode))
        {
            char filename[PATH_MAX];
            snprintf(filename, sizeof(filename), "%s.tmp", options.output_path);
            strncpy(temp_path, filename, sizeof(temp_path));
        }
        else
        {
            mkdir(temp_path, 0755);
        }

        // Primero comprimir al temporal
        if (stat(current_input, &st) == 0 && S_ISDIR(st.st_mode))
        {
            printf("\n[MODE] Directory compression (concurrent)\n");
            printf("Source directory : %s\n", current_input);
            printf("Target directory : %s\n\n", temp_path);
            int rc = compress_directory_rle_with_report(current_input, temp_path);
            if (rc != 0)
            {
                fprintf(stderr, "Directory compression failed.\n");
                if (needs_cleanup)
                    unlink(temp_path);
                return rc;
            }
            printf("\nCompression completed successfully.\n");
        }
        else
        {
            printf("\n[MODE] Single file compression\n");
            int rc = compress_file_rle_with_report(current_input, temp_path);
            if (rc != 0)
            {
                fprintf(stderr, "File compression failed.\n");
                if (needs_cleanup)
                    unlink(temp_path);
                return rc;
            }
            printf("\nCompression completed successfully.\n");
        }

        // Luego encriptar del temporal al destino final
        current_input = temp_path;
        if (!options.key)
        {
            fprintf(stderr, "Encryption requires a key (-k option)\n");
            if (needs_cleanup)
                unlink(temp_path);
            return 2;
        }

        if (stat(current_input, &st) == 0 && S_ISDIR(st.st_mode))
        {
            printf("\n[MODE] Directory encryption (concurrent)\n");
            printf("Source directory : %s\n", current_input);
            printf("Target directory : %s\n\n", final_output);
            int rc = encrypt_directory_with_report(current_input, final_output, options.key);
            if (rc != 0)
            {
                fprintf(stderr, "Directory encryption failed.\n");
                if (needs_cleanup)
                    unlink(temp_path);
                return rc;
            }
            printf("\nDirectory encryption completed successfully.\n");
        }
        else
        {
            printf("\n[MODE] Single file encryption\n");
            int rc = encrypt_file_with_report(current_input, final_output, options.key);
            if (rc != 0)
            {
                fprintf(stderr, "Encryption failed.\n");
                if (needs_cleanup)
                    unlink(temp_path);
                return rc;
            }
            printf("\nEncryption completed successfully.\n");
        }
    }
    else
    {
        // Operaciones simples sin archivos temporales
        if (has_flag(options.operation, 'c'))
        {
            if (stat(current_input, &st) == 0 && S_ISDIR(st.st_mode))
            {
                printf("\n[MODE] Directory compression (concurrent)\n");
                printf("Source directory : %s\n", current_input);
                printf("Target directory : %s\n\n", final_output);
                int rc = compress_directory_rle_with_report(current_input, final_output);
                if (rc != 0)
                {
                    fprintf(stderr, "Directory compression failed.\n");
                    return rc;
                }
            }
            else
            {
                printf("\n[MODE] Single file compression\n");
                int rc = compress_file_rle_with_report(current_input, final_output);
                if (rc != 0)
                {
                    fprintf(stderr, "File compression failed.\n");
                    return rc;
                }
            }
            printf("\nCompression completed successfully.\n");
        }

        if (has_flag(options.operation, 'd'))
        {
            if (stat(current_input, &st) == 0 && S_ISDIR(st.st_mode))
            {
                printf("\n[MODE] Directory decompression (concurrent)\n");
                printf("Source directory : %s\n", current_input);
                printf("Target directory : %s\n\n", final_output);
                int rc = decompress_directory_rle_with_report(current_input, final_output);
                if (rc != 0)
                {
                    fprintf(stderr, "Directory decompression failed.\n");
                    return rc;
                }
            }
            else
            {
                printf("\n[MODE] Single file decompression\n");
                int rc = decompress_file_rle_with_report(current_input, final_output);
                if (rc != 0)
                {
                    fprintf(stderr, "Decompression failed.\n");
                    return rc;
                }
            }
            printf("\nDecompression completed successfully.\n");
        }

        if (has_flag(options.operation, 'e'))
        {
            if (!options.key)
            {
                fprintf(stderr, "Encryption requires a key (-k option)\n");
                return 2;
            }

            if (stat(current_input, &st) == 0 && S_ISDIR(st.st_mode))
            {
                printf("\n[MODE] Directory encryption (concurrent)\n");
                printf("Source directory : %s\n", current_input);
                printf("Target directory : %s\n\n", final_output);
                int rc = encrypt_directory_with_report(current_input, final_output, options.key);
                if (rc != 0)
                {
                    fprintf(stderr, "Directory encryption failed.\n");
                    return rc;
                }
            }
            else
            {
                printf("\n[MODE] Single file encryption\n");
                int rc = encrypt_file_with_report(current_input, final_output, options.key);
                if (rc != 0)
                {
                    fprintf(stderr, "Encryption failed.\n");
                    return rc;
                }
            }
            printf("\nEncryption completed successfully.\n");
        }

        if (has_flag(options.operation, 'u'))
        {
            if (!options.key)
            {
                fprintf(stderr, "Decryption requires a key (-k option)\n");
                return 2;
            }

            if (stat(current_input, &st) == 0 && S_ISDIR(st.st_mode))
            {
                printf("\n[MODE] Directory decryption (concurrent)\n");
                printf("Source directory : %s\n", current_input);
                printf("Target directory : %s\n\n", final_output);
                int rc = decrypt_directory_with_report(current_input, final_output, options.key);
                if (rc != 0)
                {
                    fprintf(stderr, "Directory decryption failed.\n");
                    return rc;
                }
            }
            else
            {
                printf("\n[MODE] Single file decryption\n");
                int rc = decrypt_file_with_report(current_input, final_output, options.key);
                if (rc != 0)
                {
                    fprintf(stderr, "Decryption failed.\n");
                    return rc;
                }
            }
            printf("\nDecryption completed successfully.\n");
        }
    }

    // Limpiar archivos temporales si fueron creados
    if (needs_cleanup)
    {
        char cmd[PATH_MAX + 100];
        snprintf(cmd, sizeof(cmd), "rm -rf %s", temp_path);
        system(cmd);
    }

    if (!has_flag(options.operation, 'c') && !has_flag(options.operation, 'd') &&
        !has_flag(options.operation, 'e') && !has_flag(options.operation, 'u'))
    {
        fprintf(stderr, "No valid operation specified.\n");
        print_help();
        return 1;
    }

    return 0;
}
