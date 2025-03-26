#ifndef ZETONY
#define ZETONY

#include "assembler.h"
#include <stdio.h>

int tkn_parse_file(FILE *file, struct Token **buf);
int tkn_parse_line(FILE *file, struct Token **buf);

#endif
