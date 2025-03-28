#include "assembler.h"
#include <stdint.h>
#include <stdlib.h>

#define MAX_MNEMONIC_SIZE 16

// match for && index into array of InstrFuncs associated with mnemonic
// order by MSByte ascending
static uint8_t g_mnemonics[][MAX_MNEMONIC_SIZE] = {
    "add",
    "ret",
    "mov",
    "movsx",
    "movzx",
};

#ifndef __SIZEOF_INT128__
#error "__uint128_t must be supported"
#endif

int instr_try_parse(char *str, Instruction **target) {
	// potrebuju implementovat hash table :[ to bude nejlepsi
	return 1;
}

typedef __uint128_t uint128_t;
