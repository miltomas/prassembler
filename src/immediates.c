#include "assembler.h"
#include <errno.h>
#include <inttypes.h>
#include <stdlib.h>

int imm_try_parse(char *str, struct Immediate *target) {
	errno = 0;
	char *endptr;

	int64_t value = strtoll(str, &endptr, 0);

	if (errno == ERANGE || *endptr != '\0')
		return 0;

	if (value >= INT8_MIN && value <= INT8_MAX) {
		target->size = BYTE;
		target->value8 = value;
	} else if (value >= INT16_MIN && value <= INT16_MAX) {
		target->size = WORD;
		target->value16 = value;
	} else if (value >= INT32_MIN && value <= INT32_MAX) {
		target->size = DWORD;
		target->value32 = value;
	} else {
		target->size = QWORD;
		target->value64 = value;
	}

	return 1;
}
