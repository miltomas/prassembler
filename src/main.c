#include "files.h"
#include "tokens.h"
#include <assert.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	struct opt_options opts;
	if (opt_options_parse(argc, argv, &opts) == EXIT_FAILURE)
		return EXIT_FAILURE;
	FILE *file_in, *file_out;
	if (files_init(&file_in, &file_out, &opts) == EXIT_FAILURE)
		return EXIT_FAILURE;

	// parse line by line rather than this
	struct Token *tokens = malloc(256 * sizeof(struct Token));
	if (tkn_parse_file(file_in, &tokens) == EXIT_FAILURE)
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}
