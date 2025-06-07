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

u_long g_tkn_line_num;
char *g_tkn_last_line;

struct tkn_ParseResult {
	u_int comment_declared : 1;
	u_int label_declared : 1;
	u_int succeded : 1;
	char *parse_end;
};

static struct tkn_TokenParser {
	u_int comment_declared : 1;
	u_long column;
} parser_state;

static const char *const g_tkn_conflict_map[] = {
	[INSTRUCTION] = "INSTRUCTION", [IMMEDIATE] = "IMMEDIATE",
	[MEMACCESS] = "MEMACCESS",	   [REGISTER] = "REGISTER",
	[PREFIX] = "PREFIX",
};

#define TKN_DECLARE_LABEL_CONFLICT(type)                                       \
type == LABEL ? "Empty label" : g_tkn_conflict_map[type]

static int tkn_label_buf_n = 128;
int g_tkn_label_buf_i;
struct Label *g_tkn_label_buf;

void tkn_add_label(struct Label label, struct tkn_ParseResult *results,
				   ETokenType type) {
	if (!results->succeded) {
		PDIAGLINE(ERR, "Malformed label! : Conflict -> %s\n",
			parser_state.column, TKN_DECLARE_LABEL_CONFLICT(type));
		g_tkn_error = 1;
		return;
	}
	if (g_tkn_label_buf_i == tkn_label_buf_n) {
		tkn_label_buf_n *= 2;
		g_tkn_label_buf = reallocarray(g_tkn_label_buf, tkn_label_buf_n,
								 sizeof(struct Label));
	}
	g_tkn_label_buf[g_tkn_label_buf_i++] = label;
}

void tkn_clear_labels() {
	for (int i = 0; i <= g_tkn_label_buf_i; i++) {
		free(g_tkn_label_buf[i].value);
	}
	g_tkn_label_buf_i = 0;
}

typedef void (*tkn_LabelCallback)(struct Label, struct tkn_ParseResult *,
								  ETokenType);

// parse basic token types (non memaccess, that could be spread out)
ETokenType tkn_parse_token(char *restrict const word,
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
		lcallback(token->label, result, token->type);
	}
	return token->type;
}

int tkn_parse_line(FILE *file, struct Token *(*tkn_buf)[TKN_LINE_MAX]) {
	size_t n = 128;
	char *cbuf = malloc(n);

	long len = getline(&cbuf, &n, file);
	if (len == -1)
		return -1;
	if ((u_long)len != strlen(cbuf)) {
		PERRLICO("Null byte detected!\n", g_tkn_line_num, len);
		return 1;
	}
	// no newline
	cbuf[len - 1] = '\0';

	g_tkn_last_line = strdup(cbuf);
	tkn_clear_labels();
	parser_state.comment_declared = 0;

	char *word, *saveptr;
	word = strtok_r(cbuf, " \t,", &saveptr);

	int tkn_i = 0;
	while (word != NULL) {
		if (tkn_i == TKN_LINE_MAX) {
			PERRLICO("Too many tokens!\n", g_tkn_line_num, (u_long)0);
			g_tkn_error = 1;
			break;
		}

		struct Token *token = (*tkn_buf)[tkn_i];
		u_long column = parser_state.column = word - cbuf;

		struct tkn_ParseResult results = {0};
		if (word[0] == '[' || word[0] == ']') {
			token->type = MEMACCESS;
			results.succeded =
				mem_try_parse(word, &token->mem /* plus save ptr */);
		} else {
			tkn_parse_token(word, &results, token, tkn_add_label);
		}

		if (results.comment_declared)
			parser_state.comment_declared = 1;
		if (parser_state.comment_declared)
			saveptr = "\0";

		if (token->type == IMMEDIATE && !results.succeded) {
			PDIAGLINE(ERR, "Malformed immediate!\n", parser_state.column);
			g_tkn_error = 1;
		}
		if (token->type == MEMACCESS && !results.succeded) {
			PDIAGLINE(ERR, "Malformed memaccess!\n", column);
			g_tkn_error = 1;
		}

		if (results.label_declared) {
			word = *(results.parse_end + 1) == '\0'
				? strtok_r(NULL, " \t", &saveptr)
				: results.parse_end + 1;
			continue;
		}

		tkn_i++;
		word = strtok_r(NULL, " \t,", &saveptr);
	}

	free(cbuf);
	g_tkn_line_num++;
	return 0;
}
