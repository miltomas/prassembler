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

#define TKN_LINE_MAX 16

// if set, output errors, exit
int g_tkn_error;

int tkn_parse_line(FILE *file, struct Token **tkn_buf
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

		if (word[0] == ';')
			break;

		char *const colon = strchr(word, ':');

		if (colon) {
			if (colon == word) {
				PERRLICO("Malformed label!", line, colon - cbuf);
				g_tkn_error = 1;
			}
			char *const next_char = colon + 1;
			const int overflow = next_char - cbuf == len;

			word = overflow || *next_char == '\0'
					   ? strtok_r(NULL, " \t", &saveptr)
					   : next_char;
			// save label to caller buffer
			// ...
			continue;
		}

		if (i == TKN_LINE_MAX) {
			PERRLICO("Too many tokens!", line, (u_long)0);
			g_tkn_error = 1;
			break;
		}

		if (word[0] == '[') {
			tkn_buf[i]->type = MEMACCESS;
			if (!mem_try_parse(word, &tkn_buf[i]->mem)) {
				PERRLICO("Malformed memaccess!", line, word - cbuf);
				g_tkn_error = 1;
			}
		} else if (isdigit(word[0])) {
			tkn_buf[i]->type = IMMEDIATE;
			if (!imm_try_parse(word, &tkn_buf[i]->imm)) {
				PERRLICO("Malformed immediate!", line, word - cbuf);
				g_tkn_error = 1;
			}
		} else if (reg_try_parse(word, &tkn_buf[i]->reg)) {
			tkn_buf[i]->type = REGISTER;
		} else if (prefix_try_parse(word, &tkn_buf[i]->prefix)) {
			tkn_buf[i]->type = PREFIX;
		} else if (instr_try_parse(word, &tkn_buf[i]->instr)) {
			tkn_buf[i]->type = INSTRUCTION;
		} else {
			tkn_buf[i]->type = LABEL;
		}

		i++;
		word = strtok_r(NULL, " \t", &saveptr);
	}

	free(cbuf);
	line++;
	return 0;
}
