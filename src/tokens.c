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

// if set, output errors, exit
int g_tkn_error;

static const char *const g_tkn_conflict_map[] = {
	[INSTRUCTION] = "INSTRUCTION", [IMMEDIATE] = "IMMEDIATE",
	[MEMACCESS] = "MEMACCESS",     [REGISTER] = "REGISTER",
	[PREFIX] = "PREFIX",
};

#define TKN_DECLARE_LABEL_CONFLICT(type)                                       \
declare_label_success ? "Empty label" : g_tkn_conflict_map[type]

int tkn_parse_line(FILE *file, struct Token *(*tkn_buf)[TKN_LINE_MAX]
				   /* label declarations */) {
	static u_long line;

	size_t n = 128;
	char *cbuf = malloc(n);

	long len = getline(&cbuf, &n, file);
	if (len == -1)
		return -1;
	if ((u_long)len != strlen(cbuf)) {
		PERRLICO("Null byte detected!\n", line, len);
		return 1;
	}
	// no newline
	cbuf[len - 1] = '\0';

	char *word, *saveptr;
	word = strtok_r(cbuf, " \t", &saveptr);

	int i = 0;
	while (word != NULL) {
		// comment - stop parsing line
		char *const comment = strchr(word, ';');
		if (comment) {
			// prematurely end word at comment
			*comment = '\0';
			// stop loop by strtok
			saveptr = "\0";
		}

		// local globals
		int declare_label;
		int declare_label_success = 0;

		char *const colon = strchr(word, ':');
		if ((declare_label = colon != NULL))
			*colon = '\0';

		if (i == TKN_LINE_MAX) {
			PERRLICO("Too many tokens!\n", line, (u_long)0);
			g_tkn_error = 1;
			break;
		}

		if (word[0] == '[') {
			(*tkn_buf)[i]->type = MEMACCESS;
			if (!mem_try_parse(word, &(*tkn_buf)[i]->mem)) {
				PERRLICO("Malformed memaccess!\n", line, word - cbuf);
				g_tkn_error = 1;
			}
		} else if (isdigit(word[0])) {
			(*tkn_buf)[i]->type = IMMEDIATE;
			if (!imm_try_parse(word, &(*tkn_buf)[i]->imm)) {
				PERRLICO("Malformed immediate!\n", line, word - cbuf);
				g_tkn_error = 1;
			}
		} else if (reg_try_parse(word, &(*tkn_buf)[i]->reg)) {
			(*tkn_buf)[i]->type = REGISTER;
		} else if (prefix_try_parse(word, &(*tkn_buf)[i]->prefix)) {
			(*tkn_buf)[i]->type = PREFIX;
		} else if (instr_try_parse(word, &(*tkn_buf)[i]->instr)) {
			(*tkn_buf)[i]->type = INSTRUCTION;
		} else {
			declare_label_success = 1;
			(*tkn_buf)[i]->type = LABEL;
		}

		if (declare_label) {
			if (!declare_label_success || colon == word) {
				PERRLICO("Malformed label! : Conflict -> %s\n", line, word - cbuf,
			 TKN_DECLARE_LABEL_CONFLICT((*tkn_buf)[i]->type));
				g_tkn_error = 1;
			}
			// save to caller buffer
			// ...
			word = *(colon + 1) == '\0' ? strtok_r(NULL, " \t", &saveptr) : colon + 1;
			continue;
		}

		i++;
		word = strtok_r(NULL, " \t", &saveptr);
	}

	free(cbuf);
	line++;
	return 0;
}
