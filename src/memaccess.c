#include "assembler.h"
#include "tokens.h"
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

// final value must not be 0 and every field may be included once
typedef enum {
	MEM_NONE = 0,
	MEM_SI = 1 << 0,
	MEM_BASE = 1 << 1,
	MEM_DIS = 1 << 2
} mem_EState;

static int mem_transition_si(struct Token *token1, struct Token *token2,
							 mem_EState _) {
	if (token1->type != TKN_IMMEDIATE) {
		struct Token *temp = token2;
		token2 = token1;
		token1 = temp;
	}

	int valid = token1->imm.size == BYTE &&
				(token1->imm.value8 == 1 || token1->imm.value8 == 2 ||
				 token1->imm.value8 == 4 || token1->imm.value8 == 8);
	if (!valid) {
		// PDIAGLINE(ERR, "scale must be 1 | 2 | 4 | 8 : got: %d",
		// token1->column, 	token1->imm.value8);
	}
	return valid;
}

struct mem_StateNode;
typedef struct mem_StateNodeTransition {
	// generic function to check tokens
	int (*transition_check)(struct Token *node1, struct Token *node2,
							mem_EState transition);
	struct mem_StateNode *node[ETOKEN_TYPE_COUNT];
	mem_EState transition_state;
} mem_StateNodeTransition;

typedef struct mem_StateNode {
	// index with operators
	struct mem_StateNodeTransition *transitions[2];
	// state given if not overriden by transition state
	mem_EState state;
} mem_StateNode;

//
// states and transitions
//

static mem_StateNode mem_node_immediate;
static mem_StateNode mem_node_register;
static mem_StateNode mem_node_label;

static mem_StateNodeTransition mem_transition_si_immediate = {
	.transition_check = mem_transition_si,
	.node = {[TKN_REGISTER] = &mem_node_register},
	.transition_state = MEM_SI};

static mem_StateNodeTransition mem_transition_si_register = {
	.transition_check = mem_transition_si,
	.node = {[TKN_IMMEDIATE] = &mem_node_immediate},
	.transition_state = MEM_SI};

static mem_StateNodeTransition mem_transition_default = {
	.transition_check = NULL,
	.node = {[TKN_REGISTER] = &mem_node_register,
			 [TKN_IMMEDIATE] = &mem_node_immediate,
			 [TKN_LABEL] = &mem_node_label},
	.transition_state = MEM_NONE};

static mem_StateNode mem_node_immediate = {
	.transitions = {&mem_transition_si_register, &mem_transition_default},
	.state = MEM_DIS};

static mem_StateNode mem_node_register = {
	.transitions = {&mem_transition_si_immediate, &mem_transition_default},
	.state = MEM_BASE};

static mem_StateNode mem_node_label = {
	.transitions = {NULL, &mem_transition_default}, .state = MEM_DIS};

//
// states and transitions
//

struct mem_ParserState {
	union {
		mem_StateNode *node;
		mem_StateNodeTransition *transition;
	};
	u_int is_transitioning;
	// validation bitmask
	u_int state;
};

int mem_try_parse(struct tkn_TokenParser *parser_state, MemAccess **target) {
	struct tkn_Arena *arena = tkn_arena_create();
	struct mem_ParserState mem_state = {.transition = &mem_transition_default,
										.is_transitioning = 1,
										.state = 0};

	char closed = 0;
	char transition_valid = 1;
	const char *saveptr = parser_state->line + parser_state->column;
	struct Token *tkn_buf[2] = {malloc(sizeof(struct Token *)),
								malloc(sizeof(struct Token *))};

	int8_t tkn_map_states[4] = {0};
	struct Token *tkn_map_tokens[4] = {0};
	int tkn_i = 0;


	char *word = tkn_word_get(parser_state, &saveptr, arena);
	if (*word != '[')
		return 0;
	while ((word = tkn_word_get(parser_state, &saveptr, arena)) != NULL) {
		
		if (*word == ']') {
			closed = 1;
			break;
		}

		struct Token *tkn = calloc(1, sizeof(struct Token));

		struct tkn_ParseResult results = {0};
		int is_op = tkn_parse_token(parser_state, word, &results, tkn,
									tkn_parser_label_add) == TKN_OPERATOR;
		if (results.label_declared)
			continue;

		transition_valid = 1;
		if (mem_state.is_transitioning) {
			mem_StateNodeTransition *t = mem_state.transition;

			mem_state.node = t->node[tkn->type];

			if (!mem_state.node)
				return 0;

			if (t->transition_check) {
				if (tkn_i == 0)
					return 0;
				transition_valid = t->transition_check(
					tkn_buf[tkn_i - 1], tkn_buf[tkn_i], t->transition_state);
			}

			mem_EState curr = mem_state.node->state;

			if (t->transition_state != MEM_NONE && tkn_i == 0)
				return 0;
			if (t->transition_state != MEM_NONE) {
				curr = t->transition_state;
				mem_EState last = tkn_map_states[tkn_i - 1];
				// restore last flag
				mem_state.state &= ~last;
			}
			// duplicate
			if (mem_state.state & curr)
				return 0;
			mem_state.state |= curr;

			tkn_map_states[tkn_i] = curr;
			tkn_map_tokens[tkn_i] = tkn;

			tkn_i++;
		} else {
			free(tkn);
			if (!is_op)
				return 0;
			if (!strchr("*+", *word))
				return 0;
			mem_state.transition = mem_state.node->transitions[*word - '*'];
			if (!mem_state.transition)
				return 0;
		}

		if (!transition_valid)
			break;
		if (results.comment_declared)
			break;

		mem_state.is_transitioning = !mem_state.is_transitioning;
	}

	tkn_arena_destroy(arena);
	return mem_state.state && closed;
}
