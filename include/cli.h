#ifndef CLI_H
#define CLI_H

typedef struct
{
    char operation[4]; // e.g. "-c", "-d", "-ce"
    char *input_path;
    char *output_path;
    char *key;
} ProgramOptions;

int parse_arguments(int argc, char *argv[], ProgramOptions *opts);
void print_help(void);

#endif
