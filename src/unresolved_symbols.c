#include "unresolved_symbols.h"
#include <sys/types.h>

static struct UnresInstrList {
	struct UnresolvedInstruction *first;
	struct UnresolvedInstruction *last;
} unres_list;

void unres_list_add(struct UnresolvedInstruction *instr) {
	if (!unres_list.first) {
		unres_list.first = instr;
		unres_list.last = instr;
		return;
	}
	unres_list.last->next = instr;
	unres_list.last = instr;
}

struct UnresolvedInstruction *unres_list_get() {
	return unres_list.first;
}
