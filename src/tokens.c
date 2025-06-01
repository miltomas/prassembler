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

static const char *const g_tkn_conflict_map[] = {
	[INSTRUCTION] = "INSTRUCTION", [IMMEDIATE] = "IMMEDIATE",
	[MEMACCESS] = "MEMACCESS",     [REGISTER] = "REGISTER",
	[PREFIX] = "PREFIX",
};

#define TKN_DECLARE_LABEL_CONFLICT(type)                                       \
type == LABEL ? "Empty label" : g_tkn_conflict_map[type]

int tkn_parse_line(FILE *file, struct Token *(*tkn_buf)[TKN_LINE_MAX],
				   int *label_n, struct Label **label_buf) {
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

	char *word, *saveptr;
	word = strtok_r(cbuf, " \t", &saveptr);

	int tkn_i = 0;
	int label_i = 0;
	while (word != NULL) {
		char *const comment = strchr(word, ';');
		if (comment) {
			// prematurely end word at comment
			*comment = '\0';
			// stop loop by strtok
			saveptr = "\0";
		}

		// LABELS
		int is_declaring_label = 0;

		char *const colon = strchr(word, ':');
		if (colon) {
			*colon = '\0';
			is_declaring_label = 1;
		}
		// LABELS

		if (tkn_i == TKN_LINE_MAX) {
			PERRLICO("Too many tokens!\n", g_tkn_line_num, (u_long)0);
			g_tkn_error = 1;
			break;
		}

		struct Token *token = (*tkn_buf)[tkn_i];

		if (word[0] == '[') {
			token->type = MEMACCESS;
			if (!mem_try_parse(word, &token->mem)) {
				PERRLICO("Malformed memaccess!\n", g_tkn_line_num, word - cbuf);
				g_tkn_error = 1;
			}
		} else if (isdigit(word[0])) {
			token->type = IMMEDIATE;
			if (!imm_try_parse(word, &token->imm)) {
				PERRLICO("Malformed immediate!\n", g_tkn_line_num, word - cbuf);
				g_tkn_error = 1;
			}
		} else if (reg_try_parse(word, &token->reg)) {
			token->type = REGISTER;
		} else if (prefix_try_parse(word, &token->prefix)) {
			token->type = PREFIX;
		} else if (instr_try_parse(word, &token->instr)) {
			token->type = INSTRUCTION;
		} else {
			token->type = LABEL;
			token->label = (struct Label){strdup(word)};
		}

		// LABELS
		if (is_declaring_label) {
			if (token->type != LABEL || colon == word) {
				PERRLICO("Malformed label! : Conflict -> %s\n", g_tkn_line_num,
			 word - cbuf, TKN_DECLARE_LABEL_CONFLICT(token->type));
				g_tkn_error = 1;
			}

			if (label_i == *label_n) {
				*label_n *= 2;
				*label_buf = reallocarray(label_buf, *label_n, sizeof(struct Label));
			}

			(*label_buf)[label_i] = (struct Label){strdup(word)};

			word = *(colon + 1) == '\0' ? strtok_r(NULL, " \t", &saveptr) : colon + 1;
			label_i++;
			continue;
		}
		// LABELS

		tkn_i++;
		word = strtok_r(NULL, " \t", &saveptr);
	}

	free(cbuf);
	g_tkn_line_num++;
	return 0;
}
