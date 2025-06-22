#include "assembler.h"
#include "symbols.h"
#include "tokens.h"
#include <stdlib.h>

static inline void free_buf(struct Token *buf[TKN_LINE_MAX], int n) {
	for (int i = 0; i < n; i++) {
		free(buf[i]);
	}
}

static inline void prep_buf(struct Token *buf[TKN_LINE_MAX], int n) {
	for (int i = 0; i < n; i++) {
		buf[i] = malloc(sizeof(struct Token));
	}
}

static inline int is_prefix_token(ETokenType type) {
    return type == TKN_PREFIX;
}

static inline int is_instruction_token(ETokenType type) {
    return type == TKN_INSTRUCTION;
}

static inline int is_operand_token(ETokenType type) {
    return type == TKN_MEMACCESS
        || type == TKN_REGISTER
        || type == TKN_IMMEDIATE
        || type == TKN_LABEL;
}

int line_form_check(struct Token *buf[TKN_LINE_MAX], int tkn_i) {
    int i = 0;
    while (i < tkn_i && is_prefix_token(buf[i]->type)) {
        i++;
    }

    if (i >= tkn_i || !is_instruction_token(buf[i]->type)) {
        return 0;
    }
    i++;

    for (; i < tkn_i; i++) {
        if (!is_operand_token(buf[i]->type)) {
            return 0;
        }
    }
    return 1;
}

// local output file position counter
uint fpos;

static inline void save_labels(struct tkn_TokenParser *parser) {
	struct Label cur;

	while ((cur = tkn_parser_label_get(parser)).value != NULL) {
		struct LabelResolution *data = malloc(sizeof(struct LabelResolution));
		data->position = fpos;
		ESymResults ret = sym_table_add(cur, data);
		
		if (ret == SYM_DUPLICATE) {
			PERRLICO("Redefinition of label!\n", parser->line_num - 1, parser->column);
			g_tkn_error = 1;
		}
		if (ret == SYM_MAX) {
			PERRLICO("Maximum labels reached!\n", parser->line_num - 1, parser->column);
			g_tkn_error = 1;
		}
	}
}

char *encoded_output;
int traverse_file(FILE *file_in) { struct Token *buf[TKN_LINE_MAX];

	struct tkn_TokenParser *parser = tkn_parser_create(file_in);

	int tkn_i;
	prep_buf(buf, TKN_LINE_MAX);
	while (tkn_parser_line(parser, &buf, &tkn_i) != -1) {
		save_labels(parser);

		if (tkn_i == 0)
			continue;

		if (!line_form_check(buf, tkn_i)) {
			PERRLICO("Malformed line form!\n", parser->line_num - 1, (u_long)0);
			g_tkn_error = 1;
		}


		free_buf(buf, tkn_i);
		prep_buf(buf, tkn_i);
	}
	free_buf(buf, TKN_LINE_MAX);

	tkn_parser_destroy(parser);
	return 0;
}
