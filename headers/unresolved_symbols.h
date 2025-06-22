#ifndef DAVINCIS_RESOLVE
#define DAVINCIS_RESOLVE

#include <sys/types.h>

struct UnresolvedInstruction {
	uint fpos;
	int token_count;
	struct UnresolvedInstruction *next;
	struct Token *tokens[];
};

void unres_list_add(struct UnresolvedInstruction *instr);
struct UnresolvedInstruction *unres_list_get();
void unres_sym_handle(struct UnresolvedInstruction *instr);
	
#endif
