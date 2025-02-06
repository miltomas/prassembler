#ifndef ARGS_PARSE
#define ARGS_PARSE

#include "assembler.h"

#define OUTPUT_FORMATS_COUNT 2

#define WARN "\033[0;33mW\033[0m:"
#define ERR "\033[0;31mE\033[0m:"

struct opt_options {
	const char *filename_in;
	const char *filename_out;
	EOutFormat format_out;
};

int opt_parse(int argc, char *argv[], struct opt_options *opts);

#endif
