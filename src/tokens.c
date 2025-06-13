#include "tokens.h"
#include "assembler.h"
#include "immediates.h"
#include "instructions.h"
#include "memaccess.h"
#include "prefixes.h"
#include "registers.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int g_tkn_error;

#define TKN_OPERATOR_CHARS "*+!$,;[]"
#define TKN_DELIMITER_CHARS " \r\t]:"

static const char *const g_tkn_conflict_map[] = {
	[TKN_INSTRUCTION] = "INSTRUCTION", [TKN_IMMEDIATE] = "IMMEDIATE",
	[TKN_MEMACCESS] = "MEMACCESS",	   [TKN_REGISTER] = "REGISTER",
	[TKN_PREFIX] = "PREFIX",
};

#define TKN_DECLARE_LABEL_CONFLICT(type)                                       \
	type == TKN_LABEL ? "Empty label" : g_tkn_conflict_map[type]

void tkn_parser_label_add(struct tkn_TokenParser *state, struct Label label,
						  struct tkn_ParseResult *results, ETokenType type) {

	if (!results->succeded) {
		PDIAGLINE(state, ERR, "Malformed label! : Conflict -> %s\n",
				  TKN_DECLARE_LABEL_CONFLICT(type));
		g_tkn_error = 1;
		return;
	}
	if (state->label_buf_i == state->label_buf_n) {
		state->label_buf_n *= 2;
		state->label_buf = reallocarray(state->label_buf, state->label_buf_n,
										sizeof(struct Label));
	}
	state->label_buf[state->label_buf_i++] = label;
}

void tkn_parser_line_clear(struct tkn_TokenParser *state) {
	if (state->line) {
		free((void *)state->line);
	}
	state->line = NULL;
	state->comment_declared = 0;
	for (int i = 0; i < state->label_buf_i; i++) {
		free(state->label_buf[i].value);
	}
	state->label_buf_i = 0;
}

typedef void (*tkn_LabelCallback)(struct tkn_TokenParser *state, struct Label,
								  struct tkn_ParseResult *, ETokenType);

// parse basic token types (non memaccess, that could be spread out)
ETokenType tkn_parse_token(struct tkn_TokenParser *state,
						   const char *restrict const word,
						   struct tkn_ParseResult *result, struct Token *token,
						   tkn_LabelCallback lcallback) {

	result->succeded = 1;

	if (*word == ';') {
		result->comment_declared = 1;
	}
	if (strchr(TKN_OPERATOR_CHARS, *word)) {
		return TKN_OPERATOR;
	}
	
	char *const colon = strchr(word, ':');
	if (colon) {
		result->label_declared = 1;
	}

	if (isdigit(word[0])) {
		token->type = TKN_IMMEDIATE;
		if (!imm_try_parse(word, &token->imm)) {
			result->succeded = 0;
		}
	} else if (reg_try_parse(word, &token->reg)) {
		token->type = TKN_REGISTER;
	} else if (prefix_try_parse(word, &token->prefix)) {
		token->type = TKN_PREFIX;
	} else if (instr_try_parse(word, &token->instr)) {
		token->type = TKN_INSTRUCTION;
	} else {
		token->type = TKN_LABEL;
		// FREE!!
		token->label = (struct Label){strdup(word)};
	}

	if (result->label_declared) {
		if (token->type != TKN_LABEL || colon == word) {
			result->succeded = 0;
		}
		lcallback(state, token->label, result, token->type);
	}

	return token->type;
}

char *tkn_parser_getline(struct tkn_TokenParser *state, size_t *n) {
	char *cbuf = malloc(*n);

	long len = getline(&cbuf, n, state->file);
	if (len == -1)
		return NULL;
	if ((u_long)len != strlen(cbuf)) {
		PERRLICO("Null byte detected!\n", state->line_num, len);
		return NULL;
	}
	// no newline
	cbuf[len - 1] = '\0';

	state->line = cbuf;
	return cbuf;
}

int tkn_parser_line(struct tkn_TokenParser *state,
					struct Token *(*tkn_buf)[TKN_LINE_MAX]) {
	tkn_parser_line_clear(state);

	size_t n = 128;
	tkn_parser_getline(state, &n);
	if (!state->line)
		return -1;

	struct tkn_Arena *word_arena = tkn_arena_create();

	const char *word, *saveptr;
	saveptr = state->line;

	int tkn_i = 0;
	while ((word = tkn_word_get(state, &saveptr, word_arena)) != NULL) {

		if (tkn_i == TKN_LINE_MAX) {
			PERRLICO("Too many tokens!\n", state->line_num, (u_long)0);
			g_tkn_error = 1;
			break;
		}

		struct Token *token = (*tkn_buf)[tkn_i];

		struct tkn_ParseResult results = {0};
		ETokenType type;

		if (word[0] == '[' || word[0] == ']') {
			type = token->type = TKN_MEMACCESS;
			int old_col = state->column;
			results.succeded = mem_try_parse(state, &token->mem);
			saveptr += state->column - old_col;
			
		} else {
			type = tkn_parse_token(state, word, &results, token,
								   tkn_parser_label_add);
		}

		if (results.comment_declared)
			state->comment_declared = 1;
		if (state->comment_declared)
			break;

		if (type == TKN_OPERATOR && *word != ',') {
			PDIAGLINE(state, ERR, "Unexpected operator\n");
			g_tkn_error = 1;
			continue;
		}

		if (token->type == TKN_IMMEDIATE && !results.succeded) {
			PDIAGLINE(state, ERR, "Malformed immediate!\n");
			g_tkn_error = 1;
		}
		if (token->type == TKN_MEMACCESS && !results.succeded) {
			PDIAGLINE(state, ERR, "Malformed memaccess!\n");
			g_tkn_error = 1;
		}

		tkn_i++;
	}

	tkn_arena_destroy(word_arena);
	state->line_num++;
	return 0;
}

struct tkn_TokenParser *tkn_parser_create(FILE *file) {
	struct tkn_TokenParser *parser = calloc(1, sizeof(struct tkn_TokenParser));
	if (!parser) {
		fprintf(stderr, ERR "Could not initialize parser!");
		return NULL;
	}
	parser->file = file;
	parser->label_buf_n = 128;
	parser->label_buf = malloc(parser->label_buf_n * sizeof(struct Label));
	return parser;
}

void tkn_parser_destroy(struct tkn_TokenParser *parser) {
	free((void *)parser->line);
	free(parser->label_buf);
	free(parser);
}

struct tkn_Arena {
	size_t n;
	char *buf;
};

struct tkn_Arena *tkn_arena_create() {
	struct tkn_Arena *arena = malloc(sizeof(struct tkn_Arena));
	arena->n = 64;
	arena->buf = malloc(arena->n);
	return arena;
}

void tkn_arena_destroy(struct tkn_Arena *arena) {
	free(arena->buf);
	free(arena);
}

char *tkn_word_get(struct tkn_TokenParser *state, const char **str,
				   struct tkn_Arena *arena) {
	while (isspace(**str)) {
		(*str)++;
	}
	if (**str == '\0')
		return NULL;

	size_t n;
	if (strchr(TKN_OPERATOR_CHARS, **str)) {
		n = 1;
	} else {
		n = strcspn(*str, TKN_OPERATOR_CHARS TKN_DELIMITER_CHARS);
		if ((*str)[n] == ':')
			n++;
	}

	if (n >= arena->n) {
		arena->n *= 2;
		arena->buf = reallocarray(arena->buf, arena->n, 1);
	}

	char *word = memcpy(arena->buf, *str, n + 1);
	word[n] = '\0';

	state->column = *str - state->line;
	*str = *str + n;
	return word;
}
