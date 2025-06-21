#include "assembler.h"
#include <string.h>

struct GperfInstr {
	const char *name;
	const struct Instruction instr;
};

const char *lookup_instructions(register const char *str, register size_t len);

int instr_try_parse(const char *str, struct Instruction *target) {
	const struct GperfInstr *gperf =
		(struct GperfInstr *)lookup_instructions(str, strlen(str));
	if (!gperf)
		return 0;
	*target = gperf->instr;
	return 1;
}
