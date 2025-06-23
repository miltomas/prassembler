#include "files.h"
#include "symbols.h"
#include "tokens.h"
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

	if (sym_table_setup())
		return EXIT_FAILURE;
	if (files_fstate_init())
		return EXIT_FAILURE;

	if (traverse_file(file_in))
		return EXIT_FAILURE;

	if (!g_tkn_error) {
		files_fstate_write(file_out);
	}

	return g_tkn_error;
}
