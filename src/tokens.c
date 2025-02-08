#include "assembler.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// podržtaška
int tkn_parse_file(FILE *file, struct Token **buf) {
	size_t n = 128;
	char *cbuf = malloc(n);

	long read;
	for (u_long line = 0; (read = getline(&cbuf, &n, file)) != -1; line++) {
		u_long len = strlen(cbuf);
		if (len != (u_long)read) {
			fprintf(stderr, ERR LICO "Null byte detected!\n", line, len);
			return EXIT_FAILURE;
		}
		// no newline
		cbuf[len - 1] = '\0';

		char *token, *saveptr;
		token = strtok_r(cbuf, " \t", &saveptr);
		while (token != NULL) {
			// avrakadabra
			token = strtok_r(NULL, " \t", &saveptr);
		}
	}
	free(cbuf);
	return EXIT_SUCCESS;
}
