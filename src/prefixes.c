#include "assembler.h"
#include <stdio.h>
#include <string.h>

struct GperfPrefix {
	const char *name;
	const struct Prefix prefix;
};

const char *lookup_prefixes(register const char *str, register size_t len);

int prefix_try_parse(const char *str, struct Prefix *target) {
	const struct GperfPrefix *gperf =
		(struct GperfPrefix *)lookup_prefixes(str, strlen(str));
	if (!gperf)
		return 0;
	*target = gperf->prefix;
	printf("%d\n", target->type);
	return 1;
}
;
