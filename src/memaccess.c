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

struct mem_Tokens {
	struct Token *tokens[4];
	int8_t states[4];
};

struct mem_ParserResults {
	char is_closed;
	char is_error;
	char is_op;
	int tkn_i;
	char *word;
	struct Token *token;
	struct mem_ParserState state;
};

int mem_transition_handle(struct mem_ParserResults *results,
						  struct mem_Tokens *tokens) {
	mem_StateNodeTransition *t = results->state.transition;

	results->state.node = t->node[results->token->type];

	if (!results->state.node)
		return 1;

	if (t->transition_check) {
		if (results->tkn_i == 0)
			return 1;
		if (t->transition_check(tokens->tokens[results->tkn_i - 1],
								tokens->tokens[results->tkn_i],
								t->transition_state))
			return 1;
	}

	mem_EState curr = results->state.node->state;

	if (t->transition_state != MEM_NONE && results->tkn_i == 0)
		return 1;
	if (t->transition_state != MEM_NONE) {
		curr = t->transition_state;
		mem_EState last = tokens->states[results->tkn_i - 1];
		// restore last flag
		results->state.state &= ~last;
	}
	// duplicate
	if (results->state.state & curr)
		return 1;
	results->state.state |= curr;

	tokens->tokens[results->tkn_i] = results->token;
	tokens->states[results->tkn_i] = curr;

	results->tkn_i++;
	return 0;
}

int mem_node_handle(struct mem_ParserResults *results) {
	// free unused token 
	free(results->token);

	if (!results->is_op)
		return 1;
	if (!strchr("*+", *results->word))
		return 1;
	results->state.transition =
		results->state.node->transitions[*results->word - '*'];
	if (!results->state.transition)
		return 1;
	return 0;
}

int mem_parser_tokens(struct tkn_TokenParser *state,
					  struct mem_Tokens *tokens) {
	struct tkn_Arena *arena = tkn_arena_create();

	struct mem_ParserState mem_state = {.transition = &mem_transition_default,
										.is_transitioning = 1,
										.state = 0};
	struct mem_ParserResults results = {.is_closed = 0,
										.is_error = 0,
										.is_op = 0,
										.tkn_i = 0,
										.word = NULL,
										.token = NULL,
										.state = mem_state};

	const char *saveptr = state->line + state->column;

	results.word = tkn_word_get(state, &saveptr, arena);
	if (*results.word != '[')
		return 0;
	while ((results.word = tkn_word_get(state, &saveptr, arena)) != NULL) {

		if (*results.word == ']') {
			results.is_closed = 1;
			break;
		}

		results.token = calloc(1, sizeof(struct Token));

		struct tkn_ParseResult tkn_parse_results = {0};
		results.is_op = tkn_parse_token(state, results.word, &tkn_parse_results,
										results.token,
										tkn_parser_label_add) == TKN_OPERATOR;

		if (tkn_parse_results.label_declared)
			continue;
		if (tkn_parse_results.comment_declared) {
			state->comment_declared = 1;
			break;
		}

		if (mem_state.is_transitioning) {
			results.is_error = mem_transition_handle(&results, tokens);
		} else {
			results.is_error = mem_node_handle(&results);
		}

		if (results.is_error)
			break;
		if (tkn_parse_results.comment_declared)
			break;

		mem_state.is_transitioning = !mem_state.is_transitioning;
	}

	tkn_arena_destroy(arena);
	return results.state.state && results.is_closed;
}
int mem_try_parse(struct tkn_TokenParser *state, MemAccess **target) {
	struct mem_Tokens tokens = {0};
	if (!mem_parser_tokens(state, &tokens))
		return 0;
	return 1;
}
