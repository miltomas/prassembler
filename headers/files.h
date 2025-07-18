#ifndef SOUBORY
#define SOUBORY

#include "options.h"
#include <stdio.h>

int files_init(FILE **file_in, FILE **file_out, struct opt_options *opts);
int files_fstate_init();
void files_fstate_write(FILE *file_out);

extern struct GlobalFileState {
	uint fpos;
	char *encoded_output;
	uint output_n;
} g_fstate;


#endif
