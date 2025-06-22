#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include "symbols.h"
#include "assembler.h"
#include <search.h>
#include <stdio.h>

static struct hsearch_data htab;

static struct sym_Keys {
	int n;
	int i;
	const char **buf;
} keys;

static void sym_keys_push(char *key) {
	if (keys.i == keys.n) {
		keys.n *= 2;
		keys.buf = realloc(keys.buf, keys.n * sizeof(char *));
		if (!keys.buf)
			perror("keys: ");
	}
	keys.buf[keys.i] = key;
	keys.i++;
}

struct LabelResolution *sym_table_find(struct Label key) {
	ENTRY q = {.key = key.value, .data = NULL};
	ENTRY *resolved_q;
	if (!hsearch_r(q, FIND, &resolved_q, &htab))
		return NULL;
	return resolved_q->data;
}

ESymResults sym_table_add(struct Label key, struct LabelResolution *data) {
	
	char *dup_key = strdup(key.value);

	ENTRY q = {.key = dup_key, .data = data};
	ENTRY *ret;
	
	if (!hsearch_r(q, ENTER, &ret, &htab))
		return SYM_MAX;
	if (dup_key != ret->key) {
		free(dup_key);
		return SYM_DUPLICATE;
	}
	sym_keys_push(dup_key);	
	return SYM_SUCCESS;
}

int sym_table_setup() {
	keys.n = 128;
	keys.buf = calloc(keys.n, sizeof(char *));
	if (!keys.buf) {
		perror("keys: ");
		return 1;
	}
	// hcreate returns 0 on error -> flip
	return !hcreate_r(SYM_MAX_LABELS, &htab); 
}

void sym_table_destroy() {
	for (int i = 0; i < keys.i; i++)
		free((void *)keys.buf[i]);
	free(keys.buf);
	hdestroy_r(&htab);
}
