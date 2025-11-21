#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cli.h"

int parse_arguments(int argc, char *argv[], ProgramOptions *opts)
{
    if (argc < 5)
        return 0; // validar que hayan al menos 5 argumentos

    memset(opts, 0, sizeof(ProgramOptions));

    for (int i = 1; i < argc; i++)
    {
        const char *arg = argv[i];
        if (arg[0] == '-' && arg[1] != 'i' && arg[1] != 'o' && arg[1] != 'k' && arg[1] != '-')
        {
            // Procesar cada caracter del argumento
            for (int j = 1; arg[j]; j++)
            {
                if (arg[j] == 'c' || arg[j] == 'd' || arg[j] == 'e' || arg[j] == 'u')
                {
                    char op[] = {arg[j], '\0'};
                    strcat(opts->operation, op);
                }
            }
        }
        else if (strcmp(argv[i], "-i") == 0 && i + 1 < argc)
        {
            opts->input_path = argv[++i];
        }
        else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc)
        {
            opts->output_path = argv[++i];
        }
        else if (strcmp(argv[i], "-k") == 0 && i + 1 < argc)
        {
            opts->key = argv[++i];
        }
        else if (strcmp(argv[i], "--help") == 0)
        {
            return 0;
        }
    }

    // validar la existencia de input, output y minimo una operacion
    if (!opts->input_path || !opts->output_path || strlen(opts->operation) == 0)
        return 0;

    return 1;
}

void print_help(void)
{
    printf("Usage: gsea [operations] -i input -o output [-k key]\n");
    printf("Operations:\n");
    printf("  -c : compress\n");
    printf("  -d : decompress\n");
    printf("  -e : encrypt\n");
    printf("  -u : decrypt\n");
    printf("You can combine them (e.g. -ce)\n");
    printf("Example: ./gsea -ce -i input.txt -o output.enc -k clave123\n");
}
