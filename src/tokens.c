#include "assembler.h"
#include "immediates.h"
#include "instructions.h"
#include "memaccess.h"
#include "prefixes.h"
#include "registers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int tkn_parse_line(FILE *file, struct Token **buf) {
	static u_long line;
	size_t n = 128;
	char *cbuf = malloc(n);

	long len = getline(&cbuf, &n, file);
	if (len == -1) {
		return -1;		
	}
	if ((u_long)len != strlen(cbuf)) {
		fprintf(stderr, ERR LICO "Null byte detected!\n", line, len);
		return 1;
	}
	// no newline
	cbuf[len - 1] = '\0';

	char *word, *saveptr;
	word = strtok_r(cbuf, " \t", &saveptr);

	for (int i = 0; word != NULL; i++) {

		if (instr_try_parse(word, &buf[i]->instr)) {
			buf[i]->type = INSTRUCTION;
		} else if (reg_try_parse(word, &buf[i]->reg)) {
			buf[i]->type = REGISTER;
		} else if (mem_try_parse(word, &buf[i]->mem)) {
			buf[i]->type = MEMACCESS;
		} else if (imm_try_parse(word, &buf[i]->imm)) {
			buf[i]->type = IMMEDIATE;
		} else if (prefix_try_parse(word, &buf[i]->prefix)) {
			buf[i]->type = PREFIX;
		} else {
			// handle comments too
			buf[i]->type = LABEL;
		}

		word = strtok_r(NULL, " \t", &saveptr);
	}

	free(cbuf);
	line++;
	return 0;
}
