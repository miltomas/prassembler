#ifndef ZETONY
#define ZETONY

#include "assembler.h"
#include <stdio.h>

#define TKN_LINE_MAX 16

int tkn_parse_file(FILE *file, struct Token *(*tkn_buf)[TKN_LINE_MAX]);
int tkn_parse_line(FILE *file, struct Token *(*tkn_buf)[TKN_LINE_MAX]);

#endif
