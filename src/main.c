#include "files.h"
#include "symbols.h"
#include "traverse.h"
#include <assert.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	struct opt_options opts;
	if (opt_options_parse(argc, argv, &opts) == EXIT_FAILURE)
		return EXIT_FAILURE;
	FILE *file_in, *file_out;
	if (files_init(&file_in, &file_out, &opts) == EXIT_FAILURE)
		return EXIT_FAILURE;

	sym_table_setup();
	if (traverse_file(file_in))
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}
