#ifndef ZETONY
#define ZETONY

#include "assembler.h"
#include <stdio.h>
#include <sys/types.h>

#define TKN_LINE_MAX 16

// if set, output errors, exit
extern int g_tkn_error;

struct tkn_TokenParser;
extern struct tkn_TokenParser *tkn_parser_create(FILE *file);
extern void tkn_parser_destroy(struct tkn_TokenParser *parser);

extern struct Label tkn_parser_label_get(struct tkn_TokenParser *state);

#define PDIAGLINE(state, diagnostic, message, ...)                             \
	fprintf(stderr,                                                            \
			diagnostic LICO message "\n%s\n"                                   \
									"%*s^\n\n",                                \
			state->line, state->column, ##__VA_ARGS__, state->last_line,       \
			(int)(state->column), "")

// free last line before calling again!
extern int tkn_parser_line(struct tkn_TokenParser *state,
						  struct Token *(*tkn_buf)[TKN_LINE_MAX]);
extern char *tkn_word_seek_end(char *word);

#endif
