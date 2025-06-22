#include "assembler.h"
#include "files.h"
#include "symbols.h"
#include "tokens.h"
#include "unresolved_symbols.h"
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

typedef void (*sym_Err_Callback)(struct UnresolvedInstruction *instr);

int validate_resolve(sym_Err_Callback scallback, struct Token **buf, int tkn_i,
					 int start_i, int once) {
	int all_fine = 1;
	for (int i = start_i; i < tkn_i; i++) {
		struct Immediate *imm_field = NULL;
		struct Label label;
		if (buf[i]->type == TKN_LABEL) {
			imm_field = &buf[i]->imm;
			label = buf[i]->label;
		} else if (buf[i]->type == TKN_MEMACCESS && buf[i]->mem->is_label) {
			imm_field = &buf[i]->mem->displacement;
			label = buf[i]->mem->label;
		}
		if (!imm_field)
			continue;

		struct LabelResolution *ret = sym_table_find(label);
		if (!ret && (all_fine || !once)) {
			struct UnresolvedInstruction *instr =
				malloc(sizeof(struct UnresolvedInstruction) +
					   tkn_i * sizeof(struct Token *));
			instr->token_count = tkn_i;
			instr->fpos = g_fstate.fpos;
			for (int i = 0; i < tkn_i; i++) {
				struct Token *token = malloc(sizeof(struct Token));
				*token = *buf[i];
				if (buf[i]->type == TKN_MEMACCESS) {
					MemAccess *memaccess = calloc(1, sizeof(MemAccess));
					token->mem = memaccess;
					*memaccess = *buf[i]->mem;
				}
				instr->tokens[i] = token;
			}

			all_fine = 0;
			scallback(instr);
		}
		if (!ret) {
			*imm_field =
				(struct Immediate){.size = QWORD, .value64 = 0};
		} else {
			*imm_field =
				(struct Immediate){.size = QWORD, .value64 = ret->position};
		}
	}
	return all_fine;
}

int validate_encode(struct tkn_TokenParser *parser,
					struct Token *buf[TKN_LINE_MAX], int tkn_i) {

	int i = 0;
	while (buf[i++]->type != TKN_INSTRUCTION) {
	}
	struct Instruction instr = buf[i - 1]->instr;

	validate_resolve(unres_list_add, buf, tkn_i, i, 1);

	if (!instr.funcs.validate(tkn_i, buf)) {
		PERRLICO("Invalid instruction form!\n", parser->line_num,
				 parser->column);
		return 0;
	}

	int bytes = 0;
	int16_t encoded = instr.funcs.encode(&bytes, buf);

	if (g_fstate.fpos + bytes >= g_fstate.output_n) {
		g_fstate.output_n *= 2;
		g_fstate.encoded_output =
			realloc(g_fstate.encoded_output, g_fstate.output_n);
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

		printf("%d\n", g_fstate.fpos);
	}
	free_buf(buf, TKN_LINE_MAX);

	struct UnresolvedInstruction *unres_instr = unres_list_get();

	while (unres_instr != NULL) {
		int i = 0;
		while (unres_instr->tokens[i++]->type != TKN_INSTRUCTION) {
		}
		struct Instruction instr = unres_instr->tokens[i - 1]->instr;

		if (!validate_resolve(unres_sym_handle, unres_instr->tokens,
							  unres_instr->token_count, i, 0)) {
			PERRLICO("Undefined label!\n", parser->line_num - 1, (u_long)0);
		}

		int bytes = 0;
		int16_t encoded = instr.funcs.encode(&bytes, unres_instr->tokens);
		memcpy(g_fstate.encoded_output + unres_instr->fpos, &encoded, bytes);

		unres_instr = unres_instr->next;
	}

	tkn_parser_destroy(parser);
	return 0;
}
