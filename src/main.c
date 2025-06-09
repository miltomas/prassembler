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

	struct Token *buf[TKN_LINE_MAX];
	for (int i = 0; i < TKN_LINE_MAX; ++i) {
		buf[i] = malloc(TKN_LINE_MAX * sizeof(struct Token));
	}
	struct tkn_TokenParser *parser = tkn_parser_create(file_in);
	while (tkn_parser_line(parser, &buf) != -1) {
	}
	tkn_parser_destroy(parser);

	return EXIT_SUCCESS;
}

// local output file position counter
uint fpos;
struct Instruction **traverse_file(void) {
		
}
