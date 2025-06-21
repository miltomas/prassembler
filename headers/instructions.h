#ifndef INSTRUKCE
#define INSTRUKCE

#include "assembler.h"
#include <stdint.h>

int instr_try_parse(const char *str, struct Instruction *target);

#endif
