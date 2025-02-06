#include "files.h"
#include <assert.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	struct opt_options opts;
	if (opt_parse(argc, argv, &opts) == EXIT_FAILURE)
		return EXIT_FAILURE;

	FILE *file_in, *file_out;
	if (init_files(&file_in, &file_out, &opts) == EXIT_FAILURE)
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}
