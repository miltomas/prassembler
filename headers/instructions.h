#ifndef INSTRUKCE
#define INSTRUKCE

#include "assembler.h"
#include <stdint.h>
// Instruction parsing interface

int instr_try_parse(char *str, struct Instruction **target);

#endif
