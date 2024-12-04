#include <stdio.h>
#include <stdlib.h>
#include "assembler.h"
#include "options.h"

enum EOutFormat output_file_format;
int main(int argc, char *argv[])
{
    if (argc != EXPECTED_ARGUMENTS_COUNT) {
        fprintf(stderr, "Invalid num of options.\n");
        return EXIT_FAILURE;
    }
    char *file_name;
    if (options_parse(argv + 1, &output_file_format, &file_name) == EXIT_FAILURE)
        return EXIT_FAILURE;
    return EXIT_SUCCESS;
}