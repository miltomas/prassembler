#include "assembler.h"
#include "files.h"
#include "symbols.h"
#include "tokens.h"
#include <stdlib.h>
#include <string.h>

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
	return type == TKN_MEMACCESS || type == TKN_REGISTER ||
		   type == TKN_IMMEDIATE || type == TKN_LABEL;
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

static inline void save_labels(struct tkn_TokenParser *parser) {
	struct Label cur;

	while ((cur = tkn_parser_label_get(parser)).value != NULL) {
		struct LabelResolution *data = malloc(sizeof(struct LabelResolution));
		data->position = g_fstate.fpos;
		ESymResults ret = sym_table_add(cur, data);

		if (ret == SYM_DUPLICATE) {
			PERRLICO("Redefinition of label '%s'!\n", parser->line_num - 1,
					 parser->column, cur.value);
			g_tkn_error = 1;
		}
		if (ret == SYM_MAX) {
			PERRLICO("Maximum labels reached!\n", parser->line_num - 1,
					 parser->column);
			g_tkn_error = 1;
		}
	}
}

static inline int validate_encode(struct tkn_TokenParser *parser,
								  struct Token *buf[TKN_LINE_MAX],
								  int tkn_i) {

	int i = 0;
	while (buf[i++]->type != TKN_INSTRUCTION) {}
	struct Instruction instr = buf[i - 1]->instr;

	for (; i < tkn_i; i++) {
		struct Immediate *imm_field = NULL;
		if (buf[i]->type == TKN_LABEL) {
			imm_field = &buf[i]->imm;
		} else if (buf[i]->type == TKN_MEMACCESS && buf[i]->mem->is_label) {
			imm_field = &buf[i]->mem->displacement;
		} 
		if (!imm_field)
			continue;

		struct LabelResolution *ret = sym_table_find(buf[i]->label);
		if (!ret) {
			// enqueue
		}
	}

	if (!instr.funcs.validate(tkn_i, buf)) {
		PERRLICO("Invalid instruction form!\n", parser->line_num, parser->column);
		return 0;
	}

	int bytes = 0;
	int16_t encoded = instr.funcs.encode(&bytes, buf);

	if (g_fstate.fpos + bytes >= g_fstate.output_n) {
		g_fstate.output_n *= 2;
		g_fstate.encoded_output = realloc(g_fstate.encoded_output, g_fstate.output_n);
	}

	memcpy(g_fstate.encoded_output, &encoded, bytes);
	
	g_fstate.fpos += bytes;
	return 1;
}

int traverse_file(FILE *file_in) {
	struct Token *buf[TKN_LINE_MAX];

	struct tkn_TokenParser *parser = tkn_parser_create(file_in);

	int tkn_i;
	prep_buf(buf, TKN_LINE_MAX);
	while (tkn_parser_line(parser, &buf, &tkn_i) != -1) {
		save_labels(parser);
		printf("%d", g_fstate.fpos);

		if (tkn_i == 0)
			continue;

		if (!line_form_check(buf, tkn_i)) {
			PERRLICO("Malformed line form!\n", parser->line_num - 1, (u_long)0);
			g_tkn_error = 1;
		}

		if (!g_tkn_error) {
			g_tkn_error = !validate_encode(parser, buf, tkn_i);
		}

		free_buf(buf, tkn_i);
		prep_buf(buf, tkn_i);
	}
	free_buf(buf, TKN_LINE_MAX);

	tkn_parser_destroy(parser);
	return 0;
}
