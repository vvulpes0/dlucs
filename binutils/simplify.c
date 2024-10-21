#include <stdlib.h>
#include <stdio.h>

#include "dlucsas.h"

static struct Child csimp(struct Child, struct DynArr, _Bool);
static int cvals(struct Node *, struct DynArr, long *, long *);
static struct Child lstsimp(struct Node *);
static struct Child opnode(struct Node *, long, long, long);
static struct Child tsimp(struct Node *, struct DynArr);

/* parent_is_arg:
 *   should be set to true if the node
 *   is a descendent of a node of type N_ArgList
 *   and theere is no branching node between them.
 */
struct Child
simplify(struct Node *n, struct DynArr ts, _Bool parent_is_arg)
{
	struct Child output;
	struct Child c;
	struct Token t;
	/* a,b for Term/Expr operations */
	long a;
	long b;
	long op;
	/* i,j for iterators */
	size_t i;
	int j=0;
	_Bool arglike = 0;
	output.tree = n;
	output.type = C_TREE;
	if (!n) return output;
	if (n->type == N_ArgList) arglike = 1;
	if (parent_is_arg && (n->num_children == 1)) arglike = 1;

	/* eradicate T_NL, T_COMMA, and T_COLON, simplify others */
	for (i = 0; i < n->num_children; ++i) {
		c = n->children[i];
		if (c.type == C_TOKEN) {
			t = ((struct Token *)(ts.contents))[c.index];
			if (t.type == T_NUM) {
				c.type = C_VALUE;
				c.value = t.value;
			}
			if ((t.type != T_NL)
			    && (t.type != T_COMMA)
			    && (t.type != T_COLON)) {
				n->children[j++] = c;
			}
		} else if (c.type == C_TREE) {
			n->children[j++] = csimp(c, ts, arglike);
		} else {
			n->children[j++] = c;
		}
	}
	n->num_children = j;

	/* lower nodes have been simplified: what can we do now? */
	switch (n->type) {
	case N_Literal:
		/* replace by singleton child */
		if (n->num_children == 1) {
			output = n->children[0];
			free(n);
			break;
		}

		/* if NOT top-level argument, ditch parens */
		if (!parent_is_arg && n->num_children == 3) {
			output = n->children[1];
			free(n);
			break;
		}
		break;
	case N_Term:
		output = tsimp(n, ts);
		break;
	case N_Expr:
		/* replace by singleton child */
		if (n->num_children == 1) {
			output = n->children[0];
			free(n);
			break;
		}

		/* operable? */
		if (n->num_children < 2) break;
		c = n->children[1];
		if (c.type != C_TOKEN) break;
		t = ((struct Token *)(ts.contents))[c.index];
		if (t.type != T_OP) break;
		op = t.value;
		if (!cvals(n,ts,&a,&b)) break;
		/* replace operation by contents */
		output = opnode(n,a,b,op);
		break;
	case N_ArgList:
		output = lstsimp(n);
		break;
	case N_Prog:
		output = lstsimp(n);
		break;
	default:
		/* no idea! */
		break;
	}
	return output;
}

static struct Child
csimp(struct Child c, struct DynArr ts, _Bool parent_is_arg)
{
	if (!ts.valid) return c;
	if (c.type != C_TREE) return c;
	return simplify(c.tree, ts, parent_is_arg);
}

static struct Child
lstsimp(struct Node *n)
{
	struct Child c;
	struct Node *p;
	union NC *block;
	c.type = C_TREE;
	c.tree = n;
	if (!n) return c;
	if (n->num_children != 2) return c;
	if (n->children[0].type != C_TREE) return c;
	p = n->children[0].tree;
	if (!p) return c;
	if (p->type != n->type) return c;
	block = realloc(p, (p->num_children + 2)*sizeof(*block));
	if (!block) return c;
	block[0].n.children = (struct Child *)&(block[1]);
	p = &(block[0].n);
	p->children[(p->num_children)++] = n->children[1];
	c.tree = p;
	free(n);
	return c;
}

static struct Child
tsimp(struct Node * n, struct DynArr ts)
{
	struct Token *tokens = ts.contents;
	struct Child c;
	struct Token t;
	long a;
	long b;
	c.type = C_TREE;
	c.tree = n;
	if (!n) return c;
	switch (n->num_children) {
	case 1: /* one child: replace with child */
		c = n->children[0];
		free(n);
		break;
	case 2:
		if (n->children[1].type == C_VALUE) {
			a = n->children[1].value;
		} else if (n->children[1].type == C_TOKEN) {
			t = tokens[n->children[1].index];
			if (t.type != T_NUM) { break; }
			a = t.value;
		} else {
			break;
		}
		if (n->children[0].type != C_TOKEN) break;
		t = tokens[n->children[0].index];
		if (t.type != T_OP) break;
		if (t.value == '+') {
			c = opnode(n, 0, a, '+');
		} else if (t.value == '-') {
			c = opnode(n, 0, a, '-');
		} else if (t.value == '~') {
			c = opnode(n, 0, ~a, '+');
		}
		break;
	case 3:
		if (!cvals(n, ts, &a, &b)) break;
		if (n->children[1].type != C_TOKEN) break;
		t = tokens[n->children[1].index];
		c = opnode(n, a, b, t.value);
		break;
	case 4:
		if (!cvals(n, ts, &a, &b)) break;
		if (n->children[1].type != C_TOKEN) break;
		t = tokens[n->children[1].index];
		c = opnode(n, a, b, t.value);
		break;
	default:
		break;
	}
	return c;
}

static int
cvals(struct Node *n, struct DynArr tL, long *a, long *b)
{
	struct Token *ts = tL.contents;
	struct Token t;
	struct Child c;
	if (!n || !a || !b) return 0;
	if (n->num_children < 2) return 0;

	c = n->children[0];
	if (c.type == C_TREE) return 0;
	if (c.type == C_VALUE) {
		*a = c.value;
	} else if (c.type == C_TOKEN) {
		t = ts[c.index];
		if (t.type == T_NUM) {
			*a = t.value;
		} else {
			return 0;
		}
	} else {
		return 0;
	}

	c = n->children[n->num_children - 1];
	if (c.type == C_TREE) return 0;
	if (c.type == C_VALUE) {
		*b = c.value;
	} else if (c.type == C_TOKEN) {
		t = ts[c.index];
		if (t.type == T_NUM) {
			*b = t.value;
		} else {
			return 0;
		}
	} else {
		return 0;
	}

	return 1;
}

static struct Child
opnode(struct Node *n, long a, long b, long op)
{
	struct Child c;
	c.tree = n;
	c.type = C_TREE;
	if (!n) return c;
	switch (op) {
	case '+':
		c.value = a + b;
		break;
	case '-':
		c.value = a - b;
		break;
	case '|':
		c.value = a | b;
		break;
	case '^':
		c.value = a ^ b;
		break;
	case '*':
		c.value = a * b;
		break;
	case '&':
		c.value = a & b;
		break;
	case '<':
		c.value = a << b;
		break;
	case '>':
		c.value = a >> b;
		break;
	case '/':
		c.value = a / b;
		break;
	default:
		return c;
		break;
	}
	c.type = C_VALUE;
	free(n);
	return c;
}
