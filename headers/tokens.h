#ifndef ZETONY
#define ZETONY

#include "assembler.h"
#include <stdio.h>
#include <sys/types.h>

#define TKN_LINE_MAX 16

// if set, output errors, exit
extern int g_tkn_error;

extern u_long g_tkn_line_num;
// saved last parsed line, not freed!
extern char *g_tkn_last_line;

#define PDIAGLINE(diagnostic, message, column, ...)                            \
	fprintf(stderr,                                                            \
			diagnostic LICO message "\n%s\n"                                   \
									"%*s^\n\n",                                \
			g_tkn_line_num, column, ##__VA_ARGS__, g_tkn_last_line,            \
			(int)(column), "")

// free last line before calling again!
extern int tkn_parse_line(FILE *file, struct Token *(*tkn_buf)[TKN_LINE_MAX]);
extern char *tkn_word_seek_end(char *word);
extern int g_tkn_label_buf_i;
extern struct Label *g_tkn_label_buf;

#endif
