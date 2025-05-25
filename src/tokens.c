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

int tkn_parse_line(FILE *file, struct Token **buf) {
	static u_long line;
	size_t n = 128;
	char *cbuf = malloc(n);

	long len = getline(&cbuf, &n, file);
	if (len == -1)
		return -1;
	if ((u_long)len != strlen(cbuf)) {
		fprintf(stderr, ERR LICO "Null byte detected!\n", line, len);
		return 1;
	}
	// no newline
	cbuf[len - 1] = '\0';

	char *word, *saveptr;
	word = strtok_r(cbuf, " \t", &saveptr);

	for (int i = 0; word != NULL; i++) {

		if (i == TKN_LINE_MAX)
			break;
		if (word[0] == ';')
			break;

		if (word[0] == '[') {
			buf[i]->type = MEMACCESS;
			if (!mem_try_parse(word, &buf[i]->mem)) {
				fprintf(stdout, LICO ERR "Malformed memaccess!", line, word - cbuf);
				g_tkn_error = 1;
			}
		}
		else if (isdigit(word[0])) {
			buf[i]->type = IMMEDIATE;
			if (!imm_try_parse(word, &buf[i]->imm)) {
				fprintf(stdout, LICO ERR "Malformed immediate!", line, word - cbuf);
				g_tkn_error = 1;
			}
		}
		else if (reg_try_parse(word, &buf[i]->reg)) {
			buf[i]->type = REGISTER;
		} else if (prefix_try_parse(word, &buf[i]->prefix)) {
			buf[i]->type = PREFIX;
		} else if (instr_try_parse(word, &buf[i]->instr)) {
			buf[i]->type = INSTRUCTION;
		} else {
			buf[i]->type = LABEL;
		}

		word = strtok_r(NULL, " \t", &saveptr);
	}

	free(cbuf);
	line++;
	return 0;
}
