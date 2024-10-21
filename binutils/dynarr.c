#include <stdlib.h>
#include <stdio.h>
#include "dlucsas.h"
#include "dlucsld.h"
#define PUSH_X(name,type) \
void push_ ## name(struct DynArr *ts, type x) { \
	type *t; \
	if (!ts) return; \
	if (!ts->valid) return; \
	t = ts->contents; \
	if (ts->size == ts->capacity) { \
		ts->capacity *= 2; \
		t = realloc(ts->contents, ts->capacity*sizeof(*t)); \
		if (!t) { \
			fprintf(stderr, \
			        "failed to allocate memory to push\n"); \
			ts->valid = 0; \
			return; \
		} \
		ts->contents = t; \
	} \
	t[ts->size++] = x; \
}
PUSH_X(child,struct Child);
PUSH_X(int,int);
PUSH_X(instr,struct Instruction);
PUSH_X(label,struct Label);
PUSH_X(token,struct Token);
PUSH_X(symbol,struct Symbol);
