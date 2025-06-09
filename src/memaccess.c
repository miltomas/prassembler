#include "assembler.h"
#include "tokens.h"
#include <string.h>
#include <sys/types.h>

// final value must not be 0 and every field may be included once
enum mem_EState {
	MEM_NONE = 0,
	MEM_SI = 1 << 0,
	MEM_BASE = 1 << 1,
	MEM_DIS = 1 << 2
};

static int mem_transition_si(struct Token *token1, struct Token *token2,
							 enum mem_EState _) {
	if (token1->type != IMMEDIATE) {
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
struct mem_StateNodeTransition {
	// generic function to check tokens
	int (*transition_check)(struct Token *node1, struct Token *node2,
							enum mem_EState transition);
	struct mem_StateNode *node[ETOKEN_TYPE_COUNT];
	enum mem_EState transition_state;
};

struct mem_StateNode {
	// index with operators
	struct mem_StateNodeTransition *transitions[2];
	// state given if not overriden by transition state
	enum mem_EState state;
};

//
// states and transitions
//

static struct mem_StateNode mem_node_immediate;
static struct mem_StateNode mem_node_register;
static struct mem_StateNode mem_node_label;

static struct mem_StateNodeTransition mem_transition_si_immediate = {
	.transition_check = mem_transition_si,
	.node = {[REGISTER] = &mem_node_register},
	.transition_state = MEM_SI};

static struct mem_StateNodeTransition mem_transition_si_register = {
	.transition_check = mem_transition_si,
	.node = {[IMMEDIATE] = &mem_node_immediate},
	.transition_state = MEM_SI};

static struct mem_StateNodeTransition mem_transition_default = {
	.transition_check = NULL,
	.node = {[REGISTER] = &mem_node_register,
			 [IMMEDIATE] = &mem_node_immediate,
			 [LABEL] = &mem_node_label},
	.transition_state = MEM_NONE};

static struct mem_StateNode mem_node_immediate = {
	.transitions = {&mem_transition_si_register, &mem_transition_default},
	.state = MEM_DIS};

static struct mem_StateNode mem_node_register = {
	.transitions = {&mem_transition_si_immediate, &mem_transition_default},
	.state = MEM_BASE};

static struct mem_StateNode mem_node_label = {
	.transitions = {NULL, &mem_transition_default}, .state = MEM_DIS};

//
// states and transitions
//

struct mem_ParserState {
	union {
		struct mem_StateNode *node;
		struct mem_StateNodeTransition *transition;
	};
	u_int is_transitioning;
	// validation bitmask
	u_int state;
};

int mem_try_parse(struct tkn_TokenParser *parser_state, MemAccess **target) {

	struct mem_ParserState mem_state = {.transition = &mem_transition_default,
										.is_transitioning = 1,
										.state = 0};

	char valid = 0;
	return 1;
}
