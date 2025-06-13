#ifndef ZETONY
#define ZETONY

#include "assembler.h"
#include <stdio.h>
#include <sys/types.h>

#define TKN_LINE_MAX 16

// if set, output errors, exit
extern int g_tkn_error;

struct tkn_TokenParser {
	FILE *file;
	const char *line;
	u_long line_num;
	u_long column;
	int comment_declared;

	int label_buf_n;
	int label_buf_i;
	struct Label *label_buf;
};

struct tkn_ParseResult {
	u_int comment_declared : 1;
	u_int label_declared : 1;
	u_int succeded : 1;
};

extern struct tkn_TokenParser *tkn_parser_create(FILE *file);
extern void tkn_parser_destroy(struct tkn_TokenParser *parser);

extern struct Label tkn_parser_label_get(struct tkn_TokenParser *state);
void tkn_parser_label_add(struct tkn_TokenParser *state, struct Label label,
						  struct tkn_ParseResult *results, ETokenType type);

#define PDIAGLINE(state, diagnostic, message, ...)                             \
	fprintf(stderr,                                                            \
			diagnostic LICO message "\n%s\n"                                   \
									"%*s^\n\n",                                \
			state->line_num, state->column, ##__VA_ARGS__, state->line,        \
			(int)(state->column), "")

// free last line before calling again!
extern int tkn_parser_line(struct tkn_TokenParser *state,
						   struct Token *(*tkn_buf)[TKN_LINE_MAX]);

struct tkn_Arena;
struct tkn_Arena *tkn_arena_create();
void tkn_arena_destroy(struct tkn_Arena *);

char *tkn_word_get(struct tkn_TokenParser *state, const char **str, struct tkn_Arena *arena);

typedef void (*tkn_LabelCallback)(struct tkn_TokenParser *state, struct Label,
								  struct tkn_ParseResult *, ETokenType);

// parse basic token types (non memaccess, that could be spread out)
ETokenType tkn_parse_token(struct tkn_TokenParser *state,
						   const char *restrict const word,
						   struct tkn_ParseResult *result, struct Token *token,
						   tkn_LabelCallback lcallback);
#endif
