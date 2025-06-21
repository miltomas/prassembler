#include "assembler.h"
#include <string.h>

struct GperfReg {
	const char *name;
	const struct Register reg;
};

const char *lookup_registers(register const char *str, register size_t len);

int reg_try_parse(const char *str, struct Register *target) {
	const struct GperfReg *gperf =
		(struct GperfReg *)lookup_registers(str, strlen(str));
	if (!gperf)
		return 0;
	*target = gperf->reg;
	return 1;
}
