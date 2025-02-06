#include "options.h"
#include <stdio.h>
#include <stdlib.h>

int init_files(FILE **file_in, FILE **file_out, struct opt_options *opts) {
	if (opts->filename_in == NULL) {
		fprintf(stderr, WARN "Input file not found! Falling back to stdin\n");
		*file_in = stdin;
	} else {
		*file_in = fopen(opts->filename_in, "r");
	}
	if (*file_in == NULL) {
		perror(ERR "Error opening input file");
		return EXIT_FAILURE;
	}

	if (opts->filename_out == NULL) {
		fprintf(stderr, WARN "Output file not found! Falling back to stdout\n");
		*file_out = stdout;
	} else {
		*file_out = fopen(opts->filename_out, "w");
	}
	if (*file_out == NULL) {
		perror(ERR "Error opening output file");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
