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

struct tkn_ParseResult {
	u_int comment_declared : 1;
	u_int label_declared : 1;
	u_int succeded : 1;
	char *parse_end;
};

struct tkn_TokenParser {
	FILE *file;
	const char *last_line;
	u_long line;
	u_long column;
	int comment_declared;

	int label_buf_n;
	int label_buf_i;
	struct Label *label_buf;
};

static const char *const g_tkn_conflict_map[] = {
	[INSTRUCTION] = "INSTRUCTION", [IMMEDIATE] = "IMMEDIATE",
	[MEMACCESS] = "MEMACCESS",     [REGISTER] = "REGISTER",
	[PREFIX] = "PREFIX",
};

#define TKN_DECLARE_LABEL_CONFLICT(type)                                       \
type == LABEL ? "Empty label" : g_tkn_conflict_map[type]

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
	if (state->last_line) {
		free((void *)state->last_line);
	}
	state->last_line = NULL;
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
						   char *restrict const word,
						   struct tkn_ParseResult *result, struct Token *token,
						   tkn_LabelCallback lcallback) {

	result->succeded = 1;

	char *const comment = strchr(word, ';');
	if (comment) {
		*comment = '\0';
		result->comment_declared = 1;
	}

	char *const colon = strchr(word, ':');
	if (colon) {
		*colon = '\0';
		result->label_declared = 1;
		result->parse_end = colon;
	}

	if (isdigit(word[0])) {
		token->type = IMMEDIATE;
		if (!imm_try_parse(word, &token->imm)) {
			result->succeded = 0;
		}
	} else if (reg_try_parse(word, &token->reg)) {
		token->type = REGISTER;
	} else if (prefix_try_parse(word, &token->prefix)) {
		token->type = PREFIX;
	} else if (instr_try_parse(word, &token->instr)) {
		token->type = INSTRUCTION;
	} else {
		token->type = LABEL;
		// FREE!!
		token->label = (struct Label){strdup(word)};
	}

	if (result->label_declared) {
		if (token->type != LABEL || colon == word) {
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
		PERRLICO("Null byte detected!\n", state->line, len);
		return NULL;
	}
	// no newline
	cbuf[len - 1] = '\0';

	state->last_line = strdup(cbuf);
	return cbuf;
}

int tkn_parser_line(struct tkn_TokenParser *state,
					struct Token *(*tkn_buf)[TKN_LINE_MAX]) {
	tkn_parser_line_clear(state);

	size_t n = 128;
	char *cbuf = tkn_parser_getline(state, &n);
	if (!cbuf)
		return -1;

	char *word, *saveptr;
	word = strtok_r(cbuf, " \t,", &saveptr);

	int tkn_i = 0;
	while (word != NULL) {
		if (tkn_i == TKN_LINE_MAX) {
			PERRLICO("Too many tokens!\n", state->line, (u_long)0);
			g_tkn_error = 1;
			break;
		}

		struct Token *token = (*tkn_buf)[tkn_i];
		state->column = word - cbuf;

		struct tkn_ParseResult results = {0};
		if (word[0] == '[' || word[0] == ']') {
			token->type = MEMACCESS;
			results.succeded = mem_try_parse(word, &token->mem, &saveptr);
		} else {
			tkn_parse_token(state, word, &results, token, tkn_parser_label_add);
		}

		if (results.comment_declared)
			state->comment_declared = 1;
		if (state->comment_declared)
			saveptr = "\0";

		if (token->type == IMMEDIATE && !results.succeded) {
			PDIAGLINE(state, ERR, "Malformed immediate!\n");
			g_tkn_error = 1;
		}
		if (token->type == MEMACCESS && !results.succeded) {
			PDIAGLINE(state, ERR, "Malformed memaccess!\n");
			g_tkn_error = 1;
		}

		if (results.label_declared) {
			word = *(results.parse_end + 1) == '\0' ? strtok_r(NULL, " \t", &saveptr)
				: results.parse_end + 1;
			continue;
		}

		tkn_i++;
		word = strtok_r(NULL, " \t,", &saveptr);
	}

	free(cbuf);
	state->line++;
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
	free((void *)parser->last_line);
	free(parser->label_buf);
	free(parser);
}

char *tkn_word_seek_end(char *word) {
	while (*word != '\0') {
		switch (*word) {
			case '\t':
			case ' ':
			case ']':
			case '*':
			case '+':
				return word;
		}
		word++;
	}
	return word;
}
