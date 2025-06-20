#include "assembler.h"
#include <stdint.h>

struct GperfInstr {
    const char *name;
	const struct Instruction instr;
};

int instr_try_parse(const char *str, struct Instruction **target) {
	return 1;
}
