#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "options.h"

char *options_output_formats[] = { "raw", "elf" };
//TODO: make options position independent
int options_parse(char *argv[], enum EOutFormat *format, char **file_name) {
    if (strcmp(argv[0], "-f")) {
        fprintf(stderr, "Output format expected!\n");
        return EXIT_FAILURE;
    }
    char format_valid = 0;
    for (int i = 0; i < OUTPUT_FORMATS_COUNT; i++) {
        if (!strcmp(argv[1], options_output_formats[i])) {
            format_valid = 1;
            *format = i;
            break;
        }
    }
    if (!format_valid) {
        fprintf(stderr, "Undefined output format!\n");
        return EXIT_FAILURE;
    }
    if (strcmp(argv[2], "-o")) {
        fprintf(stderr, "Output file expected!\n");
        return EXIT_FAILURE;
    }
    *file_name = argv[3];
    return EXIT_SUCCESS;
}