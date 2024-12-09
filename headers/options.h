#ifndef ARGS_PARSE
#define ARGS_PARSE

#include <stdlib.h>
#include "assembler.h"

#define EXPECTED_ARGUMENTS_COUNT 5
#define OUTPUT_FORMATS_COUNT 2

extern char *options_output_formats[];
int options_parse(char *argv[], EOutFormat *format, char **file_name);

#endif
