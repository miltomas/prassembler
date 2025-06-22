#ifndef SYMBOLIKA
#define SYMBOLIKA

#include "assembler.h"

#define SYM_MAX_LABELS 1024

typedef enum {
	SYM_MAX,
	SYM_SUCCESS,
	SYM_DUPLICATE
} ESymResults;

struct LabelResolution {
	uint position;
};

struct LabelResolution *sym_table_find(struct Label key);
ESymResults sym_table_add(struct Label key, struct LabelResolution *data);
int sym_table_setup(void);

#endif
