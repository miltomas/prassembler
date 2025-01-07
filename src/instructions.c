#include <stdint.h>

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

typedef __uint128_t uint128_t;