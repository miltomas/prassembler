#include "assembler.h"

int instr_mov_validate(int tkn_count, struct Token *tokens) {
	return 1;
}
int16_t instr_mov_encode(struct Instruction *instr) {
	return 32;
}
