#define _GNU_SOURCE
#include "symbols.h"
#include "assembler.h"
#include <search.h>
#include <stdio.h>

static struct hsearch_data htab;
struct LabelResolution *sym_table_find(struct Label key) {
	ENTRY q = {.key = key.value, .data = NULL};
	ENTRY *resolved_q;
	if (!hsearch_r(q, FIND, &resolved_q, &htab))
		return NULL;
	return resolved_q->data;
}

ESymResults sym_table_add(struct Label key, struct LabelResolution *data) {
	ENTRY q = {.key = key.value, .data = data};
	ENTRY *ret;
	if (!hsearch_r(q, ENTER, &ret, &htab))
		return SYM_MAX;
	if (q.key != ret->key) {
		return SYM_DUPLICATE;
	}
	return SYM_SUCCESS;
}

int sym_table_setup() { return hcreate_r(SYM_MAX_LABELS, &htab); }
