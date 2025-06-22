#include "files.h"
#include "symbols.h"
#include "tokens.h"
#include <assert.h>
#include <stdlib.h>

int traverse_file(FILE *file_in);
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

static inline void free_buf(struct Token *buf[TKN_LINE_MAX], int n) {
	for (int i = 0; i < n; i++) {
		free(buf[i]);
	}
}

static inline void prep_buf(struct Token *buf[TKN_LINE_MAX], int n) {
	for (int i = 0; i < n; i++) {
		buf[i] = malloc(n * sizeof(struct Token));
	}
}

// local output file position counter
uint fpos;
int traverse_file(FILE *file_in) {
	struct Token *buf[TKN_LINE_MAX];

	struct tkn_TokenParser *parser = tkn_parser_create(file_in);

	int tkn_i;
	prep_buf(buf, TKN_LINE_MAX);
	while (tkn_parser_line(parser, &buf, &tkn_i) != -1) {
		struct Label cur;
		while ((cur = tkn_parser_label_get(parser)).value != NULL) {
			struct LabelResolution *data = malloc(sizeof(struct LabelResolution));
			data->position = fpos;
			sym_table_add(cur, data);
		}
		
		free_buf(buf, tkn_i);
		prep_buf(buf, tkn_i);
	}
	free_buf(buf, TKN_LINE_MAX);

	tkn_parser_destroy(parser);
	return 0;
}
