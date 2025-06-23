#include "assembler.h"

int instr_mov_validate(int tkn_count, struct Token **tokens) {
	return 1;
}
int instr_mov_encode(void *buf, struct Token **tokens) {
	*((uint32_t *)buf) = 0x656565;
	return 3;
}
