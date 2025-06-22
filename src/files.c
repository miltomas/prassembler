#include "options.h"
#include "files.h"
#include <stdio.h>
#include <stdlib.h>

struct GlobalFileState g_fstate;

int files_fstate_init() {
	g_fstate.output_n = 1024;
	g_fstate.encoded_output = malloc(g_fstate.output_n);
	if (!g_fstate.encoded_output) {
		perror(ERR "Uh oh");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int files_init(FILE **file_in, FILE **file_out, struct opt_options *opts) {
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
