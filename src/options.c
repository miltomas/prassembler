#include "options.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int opt_options_parse(int argc, char *argv[], struct opt_options *opts) {
	char *opts_out_formats[] = {[RAW] = "raw", [ELF] = "elf"};
	char *format = NULL;

	opts->filename_in = NULL;
	opts->filename_out = NULL;

	int c;
	while ((c = getopt(argc, argv, ":o:f:")) != -1) {
		switch (c) {
		case 'o':
			opts->filename_out = (char *)optarg;
			break;
		case 'f':
			format = (char *)optarg;
			break;
		case ':':
			fprintf(stderr, ERR "Option '%c' requires an argument!\n", optopt);
			return EXIT_FAILURE;
		case '?':
			fprintf(stderr, ERR "Unknown option '-%c'!\n", optopt);
			return EXIT_FAILURE;
		default:
			// NIKDY!1!
			assert(0);
		}
	}
	if (argc - optind > 1) {
		fprintf(stderr, ERR "Too many input files! Expected: 1; Got: %d\n",
				argc - optind);
		return EXIT_FAILURE;
	}
	opts->filename_in = argv[optind];

	if (format == NULL) {
		fprintf(stderr, ERR "Format '-f [\"raw\" | \"elf\"]' required!\n");
		return EXIT_FAILURE;
	}
	char format_valid = 0;
	// EOutFormat value format mandated
	for (int i = RAW; i < RAW + OUTPUT_FORMATS_COUNT; i++) {
		if (!strcmp(format, opts_out_formats[i])) {
			opts->format_out = (EOutFormat)i;
			format_valid = 1;
			break;
		}
	}
	if (!format_valid) {
		fprintf(stderr, ERR "Unrecognized out format '%s'\n", format);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
