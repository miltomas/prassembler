#include "assembler.h"

int instr_mov_validate(int tkn_count, struct Token **tokens) {
	return 1;
}
int16_t instr_mov_encode(int *byte_count, struct Token **tokens) {
	*byte_count = 1;
	return 32;
}
