#include "assembler.h"
#include "tokens.h"
#include <sys/types.h>

// final value must not be 0 and every field may be included once
enum mem_EState {
	MEM_NONE = 0,
	MEM_SI = 1 << 0,
	MEM_REG = 1 << 1,
	MEM_IMM = 1 << 2
};

static int mem_transition_si(struct Token *token1, struct Token *token2,
							 enum mem_EState _) {
	if (token2->type == IMMEDIATE) {
		struct Token *temp = token2;
		token2 = token1;
		token1 = temp;
	}

	int valid = token1->imm.size == BYTE;
	for (int i = 0; i < 4; i++) {
		if ((1 << i) == token1->imm.value8) {
			valid &= 1;
		}
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
	.node =
	{
		[REGISTER] = &mem_node_register,
		[IMMEDIATE] = &mem_node_immediate,
	},
	.transition_state = MEM_NONE};

static struct mem_StateNode mem_node_immediate = {
	.transitions = {&mem_transition_si_register, &mem_transition_default},
	.state = MEM_IMM};

static struct mem_StateNode mem_node_register = {
	.transitions = {&mem_transition_si_immediate, &mem_transition_default},
	.state = MEM_REG};

//
// states and transitions
//

int mem_try_parse(char *str, MemAccess **target, char *saveptr) {
	int i = 0;
	int c = 0;

	enum mem_EState state_node;
	if (str[0] != '[') {
		return 0;
	}

	while ((c = str[++i]) != '\0') {
		/*
	 * if label => mem_ParsingState.displacement = 1;
	 * if register =>
	 */
	}
}
