#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "options.h"

char *options_output_formats[] = { 
  [RAW] = "raw",
  [ELF] = "elf"
};
//TODO: make options position independent
int options_parse(char *argv[], EOutFormat *format, char **file_name)
{
    if (strcmp(argv[0], "-f")) {
        fprintf(stderr, "Output format expected!\n");
        return EXIT_FAILURE;
    }
    char format_valid = 0;
    for (int i = RAW; i < RAW + OUTPUT_FORMATS_COUNT; i++) {
        if (!strcmp(argv[1], options_output_formats[i])) {
            format_valid = 1;
            *format = (EOutFormat)i;
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
    *file_name = strcmp(argv[3], "-") ? argv[3] : NULL;
    return EXIT_SUCCESS;
}
