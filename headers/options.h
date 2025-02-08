#ifndef ARGS_PARSE
#define ARGS_PARSE

#include "assembler.h"

#define OUTPUT_FORMATS_COUNT 2

struct opt_options {
	const char *filename_in;
	const char *filename_out;
	EOutFormat format_out;
};

int opt_options_parse(int argc, char *argv[], struct opt_options *opts);

#endif
