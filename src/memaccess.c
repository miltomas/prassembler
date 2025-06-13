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
	/*

	struct tkn_Arena *arena = tkn_arena_create();
	struct mem_ParserState mem_state = {.transition = &mem_transition_default,
										.is_transitioning = 1,
										.state = 0};

	char closed = 0;
	char transition_valid = 1;
	const char *saveptr = parser_state->line + parser_state->column;
	struct Token *tkns[2] = {malloc(sizeof(struct Token *)),
							 malloc(sizeof(struct Token *))};
	mem_EState states[2] = {MEM_NONE, MEM_NONE};
	char *word;
	while ((word = tkn_word_get(parser_state, &saveptr, arena)) != NULL) {
		if (*word == ']' || !transition_valid) {
			closed = 1;
			break;
		}
		struct tkn_ParseResult results = {0};
		transition_valid = 1;
		if (mem_state.is_transitioning) {
			mem_StateNodeTransition *t = mem_state.transition;
			tkn_parse_token(parser_state, word, &results, tkns[1],
							tkn_parser_label_add);
			mem_state.node = t->node[tkns[1]->type];

			transition_valid =
				t->transition_check(tkns[0], tkns[1], t->transition_state);
			mem_EState cur =
				t->transition_state == MEM_NONE ? t->transition_state;
		}

		if (!transition_valid)
			break;
		if (results.comment_declared)
			break;

		char *word = tkn_word_get(parser_state, &saveptr, arena);
	}

	tkn_arena_destroy(arena);
	return 1;
	*/
}
