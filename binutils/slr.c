#include <stdio.h>
#include <stdlib.h>

#include "dlucsas.h"

struct Action {
	_Bool is_state;
	union {
		int state;
		int (*action)(struct DynArr *);
	};
};

static struct Action actions[][18];
static int go[][6];
static int r01(struct DynArr *);
static int r02(struct DynArr *);
static int r03(struct DynArr *);
static int r04(struct DynArr *);
static int r05(struct DynArr *);
static int r06(struct DynArr *);
static int r07(struct DynArr *);
static int r08(struct DynArr *);
static int r09(struct DynArr *);
static int r10(struct DynArr *);
static int r11(struct DynArr *);
static int r12(struct DynArr *);
static int r13(struct DynArr *);
static int r14(struct DynArr *);
static int r15(struct DynArr *);
static int r16(struct DynArr *);
static int r17(struct DynArr *);
static int r18(struct DynArr *);
static int r19(struct DynArr *);
static int r20(struct DynArr *);
static int r21(struct DynArr *);
static int r22(struct DynArr *);
static int r23(struct DynArr *);
static int r24(struct DynArr *);
static int r25(struct DynArr *);

static int
la_column(struct Token t)
{
	if (t.type == T_WORD)   return  0;
	if (t.type == T_NUM)    return  1;
	if (t.type == T_COLON)  return 12;
	if (t.type == T_COMMA)  return 13;
	if (t.type == T_NL)     return 14;
	if (t.type == T_OPENP)  return 15;
	if (t.type == T_CLOSEP) return 16;
	if (t.type == T_END)    return 17;
	if (t.value == '+')     return  2;
	if (t.value == '-')     return  3;
	if (t.value == '|')     return  4;
	if (t.value == '^')     return  5;
	if (t.value == '*')     return  6;
	if (t.value == '&')     return  7;
	if (t.value == '<')     return  8;
	if (t.value == '>')     return  9;
	if (t.value == '/')     return 10;
	if (t.value == '~')     return 11;
	fprintf(stderr,
	        "unknown operation token \"%c\" "
	        "at line %zu, column %zu\n",
	        (char)(t.value), t.line, t.column);
	return 0;
}

static int
go_column(enum NodeType t)
{
	if (t == N_Prog) return 0;
	if (t == N_Instruction) return 1;
	if (t == N_ArgList) return 2;
	if (t == N_Expr) return 3;
	if (t == N_Term) return 4;
	if (t == N_Literal) return 5;
	fprintf(stderr, "unknown node type: %d\n", t);
	return 0;
}

void
free_tree(struct Node * node)
{
	size_t i;
	if (!node) return;
	for (i = 0; i < node->num_children; ++i) {
		if (node->children[i].type == C_TREE) {
			free_tree(node->children[i].tree);
		}
	}
	free(node);
}

static void
free_trees(struct DynArr ts)
{
	struct Child * cs = ts.contents;
	size_t i;
	for (i = 0; i < ts.size; ++i) {
		if (cs[i].type == C_TREE) free_tree(cs[i].tree);
	}
}

static void
perr_expected(int state)
{
	fprintf(stderr, "expected ");
	switch (state) {
	case 0:
		fprintf(stderr, "label or instruction");
		break;
	case 1:
		fprintf(stderr, "expression, ':', or newline");
		break;
	case 2:
	case 3:
	case 20:
	case 43:
	case 46:
	case 47:
	case 48:
		fprintf(stderr, "label, instruction, or end of input");
		break;
	case 4:
		fprintf(stderr, "instruction");
		break;
	case 5:
	case 6:
	case 7:
	case 26:
	case 27:
	case 30:
	case 40:
	case 41:
		fprintf(stderr, "value");
		break;
	case 8:
	case 13:
	case 21:
	case 22:
	case 23:
	case 24:
	case 25:
		fprintf(stderr, "expression");
		break;
	case 9:
	case 31:
		fprintf(stderr, "',' or newline");
		break;
	case 10:
	case 33:
		fprintf(stderr, "binary operator");
		break;
	case 11:
	case 12:
	case 14:
	case 15:
	case 16:
	case 17:
	case 18:
	case 19:
	case 32:
	case 34:
	case 35:
	case 36:
	case 37:
	case 38:
	case 39:
	case 42:
	case 44:
	case 45:
		fprintf(stderr, "binary operator, ',', or ')'");
		break;
	case 28:
		fprintf(stderr, "'<'");
		break;
	case 29:
		fprintf(stderr, "'>'");
		break;
	default:
		break;
	}
	fprintf(stderr, "\n");
}

struct Node *
parse(struct DynArr const tokens, char const * str)
{
	/* initialize */
	struct Token const * const ts = tokens.contents;
	struct Token t;
	struct Child *treeList;
	struct Node *last;
	int *sts;
	struct DynArr trees;
	struct DynArr states;
	struct Action action;
	struct Child child;
	size_t i = 0;
	int state = 0;
	int n;
	if (!tokens.valid) return NULL;
	trees.capacity = 4096;
	trees.size = 0;
	states.capacity = 4096;
	states.size = 0;
	trees.contents = malloc(trees.capacity * sizeof(struct Child));
	if (!trees.contents) return NULL;
	trees.valid = 1;
	states.contents = malloc(states.capacity * sizeof(int));
	sts = states.contents;
	if (!states.contents) {
		free(trees.contents);
		return NULL;
	}
	states.valid = 1;
	sts[0] = 0;
	states.size = 1;

	/* shift-reduce loop */
	while ((state >= 0) && (i < tokens.size) && trees.valid) {
		action = actions[state][la_column(ts[i])];
		if (action.is_state && (action.state == 0)) {
			treeList = trees.contents;
			last = treeList[trees.size - 1].tree;
			free(trees.contents);
			free(states.contents);
			return last;
		} else if (action.is_state) {
			child.type = C_TOKEN;
			child.index = i;
			push_child(&trees, child);
			push_int(&states, action.state);
			sts = states.contents;
			++i;
		} else {
			n = action.action(&trees);
			if ((n < 0) || (states.size < (size_t)(n) + 1)) {
				fprintf(stderr,"- bad reduce\n");
				free_trees(trees);
				trees.contents = NULL;
				trees.valid = 0;
				free(states.contents);
				return NULL;
			}
			treeList = trees.contents;
			last = treeList[trees.size - 1].tree;
			states.size -= n;
			state = sts[states.size - 1];
			state = go[state][go_column(last->type)];
			push_int(&states,state);
			sts = states.contents;
		}
		state = sts[states.size - 1];
	}
	if (state >= 0 && trees.valid) {
		fprintf(stderr, "unexpected end of input\n");
	} else if (state < 0) {
		t = ts[i - 1];
		print_err();
		if (t.type == T_NL) {
			fprintf(stderr,
			        "unexpected newline at line %zu, "
			        "column %zu\n",
			        t.line, t.column);
		} else {
			fprintf(stderr,
			        "unexpected token \"%.*s\" "
			        "at line %zu, column %zu\n",
			        (int)(t.length), str + t.start,
			        t.line, t.column);
		}
		if (states.size > 1) {
			fprintf(stderr, " * ");
			perr_expected(sts[states.size - 2]);
		}
		perr_locus(t, str);
	} else {
		fprintf(stderr, "internal error\n");
	}
	free(trees.contents);
	free(states.contents);
	return NULL;
}

static struct Action actions[49][18] = {
	/*  0 */
	{{1,          1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1}},
	/*  1 */
	{{1,         15},{1,         14},{1,          6},{1,          5},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,          7},
	 {1,          4},{1,         -1},{1,         47},{1,          8},
	 {1,         -1},{1,         -1}},
	/*  2 */
	{{0,.action=r01},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{0,.action=r01}},
	/*  3 */
	{{1,          1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,          0}},
	/*  4 */
	{{1,         13},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1}},
	/*  5 */
	{{1,         15},{1,         14},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,          8},
	 {1,         -1},{1,         -1}},
	/*  6 */
	{{1,         15},{1,         14},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,          8},
	 {1,         -1},{1,         -1}},
	/*  7 */
	{{1,         15},{1,         14},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,          8},
	 {1,         -1},{1,         -1}},
	/*  8 */
	{{1,         15},{1,         14},{1,          6},{1,          5},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,          7},
	 {1,         -1},{1,         -1},{1,         -1},{1,          8},
	 {1,         -1},{1,         -1}},
	/*  9 */
	{{1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         21},{1,         20},{1,         -1},
	 {1,         -1},{1,         -1}},
	/* 10 */
	{{1,         -1},{1,         -1},{1,         22},{1,         23},
	 {1,         24},{1,         25},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{0,.action=r05},{0,.action=r05},{1,         -1},
	 {1,         -1},{1,         -1}},
	/* 11 */
	{{1,         -1},{1,         -1},{0,.action=r07},{0,.action=r07},
	 {0,.action=r07},{0,.action=r07},{1,         26},{1,         27},
	 {1,         28},{1,         29},{1,         29},{1,         -1},
	 {1,         -1},{0,.action=r07},{0,.action=r07},{1,         -1},
	 {0,.action=r07},{1,         -1}},
	/* 12 */
	{{1,         -1},{1,         -1},{0,.action=r17},{0,.action=r17},
	 {0,.action=r17},{0,.action=r17},{0,.action=r17},{0,.action=r17},
	 {0,.action=r17},{0,.action=r17},{0,.action=r17},{1,         -1},
	 {1,         -1},{0,.action=r17},{0,.action=r17},{1,         -1},
	 {0,.action=r17},{1,         -1}},
	/* 13 */
	{{1,         15},{1,         14},{1,          6},{1,          5},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,          7},
	 {1,         -1},{1,         -1},{1,         48},{1,          8},
	 {1,         -1},{1,         -1}},
	/* 14 */
	{{1,         -1},{1,         -1},{0,.action=r21},{0,.action=r21},
	 {0,.action=r21},{0,.action=r21},{0,.action=r21},{0,.action=r21},
	 {0,.action=r21},{0,.action=r21},{0,.action=r21},{1,         -1},
	 {1,         -1},{0,.action=r21},{0,.action=r21},{1,         -1},
	 {0,.action=r21},{1,         -1}},
	/* 15 */
	{{1,         -1},{1,         -1},{0,.action=r22},{0,.action=r22},
	 {0,.action=r22},{0,.action=r22},{0,.action=r22},{0,.action=r22},
	 {0,.action=r22},{0,.action=r22},{0,.action=r22},{1,         -1},
	 {1,         -1},{0,.action=r22},{0,.action=r22},{1,         -1},
	 {0,.action=r22},{1,         -1}},
	/* 16 */
	{{1,         -1},{1,         -1},{0,.action=r18},{0,.action=r18},
	 {0,.action=r18},{0,.action=r18},{0,.action=r18},{0,.action=r18},
	 {0,.action=r18},{0,.action=r18},{0,.action=r18},{1,         -1},
	 {1,         -1},{0,.action=r18},{0,.action=r18},{1,         -1},
	 {0,.action=r18},{1,         -1}},
	/* 17 */
	{{1,         -1},{1,         -1},{0,.action=r19},{0,.action=r19},
	 {0,.action=r19},{0,.action=r19},{0,.action=r19},{0,.action=r19},
	 {0,.action=r19},{0,.action=r19},{0,.action=r19},{1,         -1},
	 {1,         -1},{0,.action=r19},{0,.action=r19},{1,         -1},
	 {0,.action=r19},{1,         -1}},
	/* 18 */
	{{1,         -1},{1,         -1},{0,.action=r20},{0,.action=r20},
	 {0,.action=r20},{0,.action=r20},{0,.action=r20},{0,.action=r20},
	 {0,.action=r20},{0,.action=r20},{0,.action=r20},{1,         -1},
	 {1,         -1},{0,.action=r20},{0,.action=r20},{1,         -1},
	 {0,.action=r20},{1,         -1}},
	/* 19 */
	{{1,         -1},{1,         -1},{1,         22},{1,         23},
	 {1,         24},{1,         25},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         32},{1,         -1}},
	/* 20 */
	{{0,.action=r04},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{0,.action=r04}},
	/* 21 */
	{{1,         15},{1,         14},{1,          6},{1,          5},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,          7},
	 {1,         -1},{1,         -1},{1,         -1},{1,          8},
	 {1,         -1},{1,         -1}},
	/* 22 */
	{{1,         15},{1,         14},{1,          6},{1,          5},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,          7},
	 {1,         -1},{1,         -1},{1,         -1},{1,          8},
	 {1,         -1},{1,         -1}},
	/* 23 */
	{{1,         15},{1,         14},{1,          6},{1,          5},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,          7},
	 {1,         -1},{1,         -1},{1,         -1},{1,          8},
	 {1,         -1},{1,         -1}},
	/* 24 */
	{{1,         15},{1,         14},{1,          6},{1,          5},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,          7},
	 {1,         -1},{1,         -1},{1,         -1},{1,          8},
	 {1,         -1},{1,         -1}},
	/* 25 */
	{{1,         15},{1,         14},{1,          6},{1,          5},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,          7},
	 {1,         -1},{1,         -1},{1,         -1},{1,          8},
	 {1,         -1},{1,         -1}},
	/* 26 */
	{{1,         15},{1,         14},{1,          6},{1,          5},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,          8},
	 {1,         -1},{1,         -1}},
	/* 27 */
	{{1,         15},{1,         14},{1,          6},{1,          5},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,          8},
	 {1,         -1},{1,         -1}},
	/* 28 */
	{{1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         40},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1}},
	/* 29 */
	{{1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         41},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1}},
	/* 30 */
	{{1,         15},{1,         14},{1,          6},{1,          5},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,          8},
	 {1,         -1},{1,         -1}},
	/* 31 */
	{{1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         21},{1,         43},{1,         -1},
	 {1,         -1},{1,         -1}},
	/* 32 */
	{{1,         -1},{1,         -1},{0,.action=r23},{0,.action=r23},
	 {0,.action=r23},{0,.action=r23},{0,.action=r23},{0,.action=r23},
	 {0,.action=r23},{0,.action=r23},{0,.action=r23},{1,         -1},
	 {1,         -1},{0,.action=r23},{0,.action=r23},{1,         -1},
	 {0,.action=r23},{1,         -1}},
	/* 33 */
	{{1,         -1},{1,         -1},{1,         22},{1,         23},
	 {1,         24},{1,         25},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{0,.action=r06},{0,.action=r06},{1,         -1},
	 {1,         -1},{1,         -1}},
	/* 34 */
	{{1,         -1},{1,         -1},{0,.action=r08},{0,.action=r08},
	 {0,.action=r08},{0,.action=r08},{1,         26},{1,         27},
	 {1,         28},{1,         29},{1,         29},{1,         -1},
	 {1,         -1},{0,.action=r08},{0,.action=r08},{1,         -1},
	 {0,.action=r08},{1,         -1}},
	/* 35 */
	{{1,         -1},{1,         -1},{0,.action=r09},{0,.action=r09},
	 {0,.action=r09},{0,.action=r09},{1,         26},{1,         27},
	 {1,         28},{1,         29},{1,         29},{1,         -1},
	 {1,         -1},{0,.action=r09},{0,.action=r09},{1,         -1},
	 {0,.action=r09},{1,         -1}},
	/* 36 */
	{{1,         -1},{1,         -1},{0,.action=r10},{0,.action=r10},
	 {0,.action=r10},{0,.action=r10},{1,         26},{1,         27},
	 {1,         28},{1,         29},{1,         29},{1,         -1},
	 {1,         -1},{0,.action=r10},{0,.action=r10},{1,         -1},
	 {0,.action=r10},{1,         -1}},
	/* 37 */
	{{1,         -1},{1,         -1},{0,.action=r11},{0,.action=r11},
	 {0,.action=r11},{0,.action=r11},{1,         26},{1,         27},
	 {1,         28},{1,         29},{1,         29},{1,         -1},
	 {1,         -1},{0,.action=r11},{0,.action=r11},{1,         -1},
	 {0,.action=r11},{1,         -1}},
	/* 38 */
	{{1,         -1},{1,         -1},{0,.action=r12},{0,.action=r12},
	 {0,.action=r12},{0,.action=r12},{0,.action=r12},{0,.action=r12},
	 {0,.action=r12},{0,.action=r12},{0,.action=r12},{1,         -1},
	 {1,         -1},{0,.action=r12},{0,.action=r12},{1,         -1},
	 {0,.action=r12},{1,         -1}},
	/* 39 */
	{{1,         -1},{1,         -1},{0,.action=r13},{0,.action=r13},
	 {0,.action=r13},{0,.action=r13},{0,.action=r13},{0,.action=r13},
	 {0,.action=r13},{0,.action=r13},{0,.action=r13},{1,         -1},
	 {1,         -1},{0,.action=r13},{0,.action=r13},{1,         -1},
	 {0,.action=r13},{1,         -1}},
	/* 40 */
	{{1,         15},{1,         14},{1,          6},{1,          5},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,          8},
	 {1,         -1},{1,         -1}},
	/* 41 */
	{{1,         15},{1,         14},{1,          6},{1,          5},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,          8},
	 {1,         -1},{1,         -1}},
	/* 42 */
	{{1,         -1},{1,         -1},{0,.action=r16},{0,.action=r16},
	 {0,.action=r16},{0,.action=r16},{0,.action=r16},{0,.action=r16},
	 {0,.action=r16},{0,.action=r16},{0,.action=r16},{1,         -1},
	 {1,         -1},{0,.action=r16},{0,.action=r16},{1,         -1},
	 {0,.action=r16},{1,         -1}},
	/* 43 */
	{{0,.action=r03},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{0,.action=r03}},
	/* 44 */
	{{1,         -1},{1,         -1},{0,.action=r14},{0,.action=r14},
	 {0,.action=r14},{0,.action=r14},{0,.action=r14},{0,.action=r14},
	 {0,.action=r14},{0,.action=r14},{0,.action=r14},{1,         -1},
	 {1,         -1},{0,.action=r14},{0,.action=r14},{1,         -1},
	 {0,.action=r14},{1,         -1}},
	/* 45 */
	{{1,         -1},{1,         -1},{0,.action=r15},{0,.action=r15},
	 {0,.action=r15},{0,.action=r15},{0,.action=r15},{0,.action=r15},
	 {0,.action=r15},{0,.action=r15},{0,.action=r15},{1,         -1},
	 {1,         -1},{0,.action=r15},{0,.action=r15},{1,         -1},
	 {0,.action=r15},{1,         -1}},
	/* 46 */
	{{0,.action=r02},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{0,.action=r02}},
	/* 47 */
	{{0,.action=r24},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{0,.action=r24}},
	/* 48 */
	{{0,.action=r25},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{1,         -1},{1,         -1},{1,         -1},
	 {1,         -1},{0,.action=r25}},
};

static int go[49][6] = {
	{ 3, 2,-1,-1,-1,-1}, /*  0 */
	{-1,-1, 9,10,11,12}, /*  1 */
	{-1,-1,-1,-1,-1,-1}, /*  2 */
	{-1,46,-1,-1,-1,-1}, /*  3 */
	{-1,-1,-1,-1,-1,-1}, /*  4 */
	{-1,-1,-1,-1,-1,16}, /*  5 */
	{-1,-1,-1,-1,-1,17}, /*  6 */
	{-1,-1,-1,-1,-1,18}, /*  7 */
	{-1,-1,-1,19,11,12}, /*  8 */
	{-1,-1,-1,-1,-1,-1}, /*  9 */
	{-1,-1,-1,-1,-1,-1}, /* 10 */
	{-1,-1,-1,-1,-1,-1}, /* 11 */
	{-1,-1,-1,-1,-1,-1}, /* 12 */
	{-1,-1,31,10,11,12}, /* 13 */
	{-1,-1,-1,-1,-1,-1}, /* 14 */
	{-1,-1,-1,-1,-1,-1}, /* 15 */
	{-1,-1,-1,-1,-1,-1}, /* 16 */
	{-1,-1,-1,-1,-1,-1}, /* 17 */
	{-1,-1,-1,-1,-1,-1}, /* 18 */
	{-1,-1,-1,-1,-1,-1}, /* 19 */
	{-1,-1,-1,-1,-1,-1}, /* 20 */
	{-1,-1,-1,33,11,12}, /* 21 */
	{-1,-1,-1,-1,34,12}, /* 22 */
	{-1,-1,-1,-1,35,12}, /* 23 */
	{-1,-1,-1,-1,36,12}, /* 24 */
	{-1,-1,-1,-1,37,12}, /* 25 */
	{-1,-1,-1,-1,-1,38}, /* 26 */
	{-1,-1,-1,-1,-1,39}, /* 27 */
	{-1,-1,-1,-1,-1,-1}, /* 28 */
	{-1,-1,-1,-1,-1,-1}, /* 29 */
	{-1,-1,-1,-1,-1,42}, /* 30 */
	{-1,-1,-1,-1,-1,-1}, /* 31 */
	{-1,-1,-1,-1,-1,-1}, /* 32 */
	{-1,-1,-1,-1,-1,-1}, /* 33 */
	{-1,-1,-1,-1,-1,-1}, /* 34 */
	{-1,-1,-1,-1,-1,-1}, /* 35 */
	{-1,-1,-1,-1,-1,-1}, /* 36 */
	{-1,-1,-1,-1,-1,-1}, /* 37 */
	{-1,-1,-1,-1,-1,-1}, /* 38 */
	{-1,-1,-1,-1,-1,-1}, /* 39 */
	{-1,-1,-1,-1,-1,44}, /* 40 */
	{-1,-1,-1,-1,-1,45}, /* 41 */
	{-1,-1,-1,-1,-1,-1}, /* 42 */
	{-1,-1,-1,-1,-1,-1}, /* 43 */
	{-1,-1,-1,-1,-1,-1}, /* 44 */
	{-1,-1,-1,-1,-1,-1}, /* 45 */
	{-1,-1,-1,-1,-1,-1}, /* 46 */
	{-1,-1,-1,-1,-1,-1}, /* 47 */
};

static int
mk(struct DynArr *stack, size_t n, enum NodeType type)
{
	union NC *block;
	struct Child *cs;
	struct Child *children;
	int i;
	if (!stack || (stack->size < n)) return -1; /* died */
	cs = stack->contents;
	block = malloc((n + 1)*sizeof(*block));
	children = (struct Child *)(&(block[1]));
	if (!block) {
		stack->valid = 0;
		return -1; /* died */
	}
	block[0].n.type = type;
	block[0].n.num_children = n;
	block[0].n.children = children;
	/* pop children into block */
	for (i = n - 1; i >= 0; --i) {
		children[i] = ((struct Child *)(cs))[--(stack->size)];
	}
	/* push new node */
	cs[stack->size].type = C_TREE;
	cs[stack->size].tree = (struct Node *)(block);
	++(stack->size);
	return n; /* successfully popped n children */
}

static int r01(struct DynArr *stack) { return mk(stack, 1, N_Prog); }
static int r02(struct DynArr *stack) { return mk(stack, 2, N_Prog); }
static int r03(struct DynArr *stack) { return mk(stack, 5, N_Instruction); }
static int r04(struct DynArr *stack) { return mk(stack, 3, N_Instruction); }
static int r05(struct DynArr *stack) { return mk(stack, 1, N_ArgList); }
static int r06(struct DynArr *stack) { return mk(stack, 3, N_ArgList); }
static int r07(struct DynArr *stack) { return mk(stack, 1, N_Expr); }
static int r08(struct DynArr *stack) { return mk(stack, 3, N_Expr); }
static int r09(struct DynArr *stack) { return mk(stack, 3, N_Expr); }
static int r10(struct DynArr *stack) { return mk(stack, 3, N_Expr); }
static int r11(struct DynArr *stack) { return mk(stack, 3, N_Expr); }
static int r12(struct DynArr *stack) { return mk(stack, 3, N_Term); }
static int r13(struct DynArr *stack) { return mk(stack, 3, N_Term); }
static int r14(struct DynArr *stack) { return mk(stack, 4, N_Term); }
static int r15(struct DynArr *stack) { return mk(stack, 4, N_Term); }
static int r16(struct DynArr *stack) { return mk(stack, 3, N_Term); }
static int r17(struct DynArr *stack) { return mk(stack, 1, N_Term); }
static int r18(struct DynArr *stack) { return mk(stack, 2, N_Term); }
static int r19(struct DynArr *stack) { return mk(stack, 2, N_Term); }
static int r20(struct DynArr *stack) { return mk(stack, 2, N_Term); }
static int r21(struct DynArr *stack) { return mk(stack, 1, N_Literal); }
static int r22(struct DynArr *stack) { return mk(stack, 1, N_Literal); }
static int r23(struct DynArr *stack) { return mk(stack, 3, N_Literal); }
static int r24(struct DynArr *stack) { return mk(stack, 2, N_Instruction); }
static int r25(struct DynArr *stack) { return mk(stack, 4, N_Instruction); }
