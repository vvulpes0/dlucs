#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "dlucsas.h"
#include "instructions.h"

static int addr_mode(struct Arg);
static void decr(struct DynArr, size_t);
static int emiti(struct Node *, struct Object *, size_t *,
                 int *, struct Token *, char const *);
static int eval(struct Node *, struct DynArr, struct Token *,
                char const *, long *);
static unsigned long ext(enum ShortType);
static int fits(long, enum ShortType);
static struct Arg get_arg(struct Node *, size_t i,
                          struct Token *, char const *);
static int get_child(struct Child, struct DynArr, struct Token *,
                     char const *, long *);
static long icond(char const *, size_t);
static struct Instruction mkbyte(struct Arg);
static struct Instruction mkword(struct Arg);
static size_t nargs(struct Node const *);
static int op(struct Node *);
static int opfind(struct Node *, struct Token *, char const *,
                  long *, int *);
static int do_label(struct Node *, struct DynArr *, size_t,
                    int, struct Token *, char const *);
static int do_instr(struct Node *, struct DynArr *, size_t *,
                    int *, struct Token *, char const *);
static int reg(struct Token, char const *);
static int strncmpi(char const *, char const *, size_t);

struct Object
emit(struct Node *prog, struct Token *ts, char const * str)
{
	struct Object obj;
	struct DynArr istream = {NULL, 0, 0, 1};
	struct DynArr labels = {NULL, 0, 0, 1};
	size_t i;
	size_t offset = 0;
	int section = 0;

	if (!prog) return (struct Object){istream, labels, 0};

	istream.capacity = 4096;
	istream.contents = malloc(istream.capacity
	                          * sizeof(struct Instruction));
	istream.size = 0;
	istream.valid = 0;
	if (!istream.contents) return (struct Object){istream, labels, 0};
	istream.valid = 1;

	labels.capacity = 4096;
	labels.contents = malloc(labels.capacity * sizeof(struct Label));
	labels.size = 0;
	labels.valid = 0;
	if (!istream.contents) {
		free(istream.contents);
		istream.size = istream.capacity = 0;
		istream.valid = 0;
		return (struct Object){istream, labels, 0};
	};
	labels.valid = 1;

	obj = (struct Object){istream, labels, 0};
	for (i = 0; i < prog->num_children; ++i) {
		if (prog->children[i].type != C_TREE) {
			destroy_obj(&obj);
			return obj;
		}
		if (!emiti(prog->children[i].tree,
		           &obj, &offset, &section,
		           ts, str)) {
			destroy_obj(&obj);
			return obj;
		}
	}
	if (section == 1) obj.ram_size = offset;
	return obj;
}

int
relax(struct Object obj, struct Token *ts, char const * str)
{
	struct Instruction *xs = obj.istream.contents;
	struct Label *ls = obj.labels.contents;
	struct Instruction *x;
	size_t offset = 0;
	size_t i = 0;
	size_t lbli;
	long value = 0;
	int changed = 0;
	if (!obj.istream.valid || !obj.labels.valid) return 0;
	for (i = 0; i < obj.istream.size; ++i) {
		x = xs + i;
		offset += x->length;
		if (x->filled || (x->length != 4)) continue;
		if (x->short_type == S_S3DE) {
			if (x->source.type != AT_Token) continue;
			lbli = lookup(obj.labels, x->source.token, str);
			if (lbli >= obj.labels.size) continue;
			if (!fits(ls[lbli].offset - offset + 2,S_S3DE)) {
				continue;
			}
			x->encoding[1] &= ~ext(S_S3DE);
			x->length = 2;
			offset -= 2;
			changed = 1;
			decr(obj.labels, offset);
			continue;
		}
		if (x->source.type != AT_Expr) continue;
		value = 0;
		if (!eval(x->source.expr, obj.labels, ts, str, &value)) {
			continue;
		}
		if (!fits(value, x->short_type)) continue;
		x->encoding[1] &= ~ext(x->short_type);
		x->length = 2;
		offset -= 2;
		changed = 1;
		decr(obj.labels, offset);
	}
	return changed;
}

void
flush(struct Object obj, struct Token *ts, char const * str)
{
	struct Instruction *xs = obj.istream.contents;
	struct Label *ls = obj.labels.contents;
	struct Instruction *x;
	size_t offset = 0;
	size_t i = 0;
	size_t lbli;
	long value = 0;
	if (!obj.istream.valid || !obj.labels.valid) return;
	for (i = 0; i < obj.istream.size; ++i) {
		x = xs + i;
		offset += x->length;
		if (x->filled) continue;
		if (x->short_type == S_S3DE) {
			if (x->source.type != AT_Token) continue;
			lbli = lookup(obj.labels, x->source.token, str);
			if (lbli >= obj.labels.size) continue;
			if (x->length == 2) {
				value = ls[lbli].offset - offset;
				value /= 2;
				x->encoding[1] |= value&7;
			} else {
				value = ls[lbli].offset - offset;
				x->encoding[2] = value&0xFF;
				x->encoding[3] = (value>>8)&0xFF;
			}
			x->filled = 1;
			continue;
		}
		if (x->source.type == AT_Value) {
			value = x->source.value;
			if (x->length == 2) {
				x->encoding[1] |= value&7;
			} else {
				x->encoding[2] = value&0xFF;
				x->encoding[3] = (value>>8)&0xFF;
			}
			x->filled = 1;
			continue;
		}
		if (x->source.type != AT_Expr) continue;
		if (!eval(x->source.expr, obj.labels, ts, str, &value)) {
			continue;
		}
		if (x->length == 2) {
			x->encoding[1] |= value&7;
		} else {
			x->encoding[2] = value&0xFF;
			x->encoding[3] = (value>>8)&0xFF;
		}
		x->filled = 1;
		continue;
	}
}

static void
decr(struct DynArr xs, size_t offset)
{
	struct Label *ls = xs.contents;
	size_t i;
	for (i = 0; i < xs.size; ++i) {
		if (ls[i].section != 0) continue;
		if (ls[i].offset < offset) continue;
		ls[i].offset -= 2;
	}
}

/* Input : ins, ts, str
 * InOut : obj, offset
 * Output: 1 if valid, 0 if invalid
 */
static int
emiti(struct Node *ins, struct Object *obj, size_t *offset,
      int *section, struct Token *ts, char const * str)
{
	if (!ins || !obj || !offset || !ts || !str) return 0;
	if (!do_label(ins, &(obj->labels), *offset, *section, ts, str)) {
		return 0;
	}
	if (!do_instr(ins, &(obj->istream), offset, section, ts, str)) {
		return 0;
	}
	return 1;
}

void
destroy_obj(struct Object *obj)
{
	if (!obj) return;
	obj->istream.size = obj->istream.capacity = 0;
	obj->istream.valid = 0;
	free(obj->istream.contents);
	obj->istream.contents = NULL;
	obj->labels.size = obj->labels.capacity = 0;
	obj->labels.valid = 0;
	free(obj->labels.contents);
	obj->labels.contents = NULL;
}

/* if found: index. if not found: size */
size_t
lookup(struct DynArr labels, struct Token t, char const * str)
{
	struct Label *ls = labels.contents;
	size_t i;
	for (i = 0; i < labels.size; ++i) {
		if (ls[i].length != t.length) continue;
		if (strncmp(str + ls[i].start,
		            str + t.start,
		            t.length) != 0) {
			continue;
		}
		return i;
	}
	return i;
}

static int
do_label(struct Node *ins, struct DynArr *labels, size_t offset,
         int section, struct Token *ts, char const * str)
{
	struct Token t;
	struct Label *ls;
	struct Label label;
	size_t nc;
	size_t i;
	if (!ins || !labels || !labels->contents || !str) return 1;
	ls = labels->contents;
	nc = ins->num_children;
	if ((nc != 2) && (nc != 3)) return 1;
	if ((nc == 2) && (ins->children[1].type == C_TREE)) return 1;
	if (ins->children[0].type != C_TOKEN) return 1;
	/* first child _should_ be T_WORD with label */
	t = ts[ins->children[0].index];
	if (t.type != T_WORD) return 1;

	/* ensure label not already present:
	 * if it is, state as much and return invalid
	 */
	i = lookup(*labels, t, str);
	if (i < labels->size) {
		print_err();
		fprintf(stderr,
		        "multiple definitions of label \"%.*s\"\n",
		        (int)(t.length), str + t.start);
		fprintf(stderr,
		        "first at line %zu, then again at line %zu\n",
		        ls[i].line, t.line);
		perr_locus(t, str);
		return 0;
	}

	/* good to go */
	label.start = t.start;
	label.length = t.length;
	label.line = t.line;
	label.offset = offset;
	label.section = section;
	push_label(labels, label);
	return labels->valid;
}

static int
addr_mode(struct Arg arg)
{
	switch (arg.type) {
	case AT_Register:
		return arg.indirect ? AM_IndRegister : AM_Register;
	case AT_Value:
	case AT_Token:
	case AT_Expr:
		return arg.indirect ? AM_IndImmediate : AM_Immediate;
	default:
		return 0;
	}
}

static size_t
nargs(struct Node const * p)
{
	if (!p || p->num_children < 1) return 0;
	if (p->children[p->num_children - 1].type != C_TREE) return 0;
	p = p->children[p->num_children - 1].tree;
	if (!p) return 0;
	return p->num_children;
}

static struct Arg
get_arg(struct Node *p, size_t i,
        struct Token *ts, char const * str)
{
	struct Arg arg = {AT_None, 0, 0};
	struct Child c;
	struct Token t;
	int r;
	if (!p | !ts | !str) return arg;
	if (p->type != N_Instruction) return arg;
	if (p->num_children == 0) return arg;
	if (p->children[p->num_children - 1].type != C_TREE) return arg;
	p = p->children[p->num_children - 1].tree;

	if (p->type != N_ArgList) return arg;
	if (p->num_children <= i) return arg;
	c = p->children[i];
	if (c.type == C_VALUE) {
		arg.value = c.value;
		arg.type = AT_Value;
		return arg;
	}
	if (c.type == C_TOKEN) {
		t = ts[c.index];
		if (t.type != T_WORD) return arg;
		r = reg(t, str);
		if (r < 0) {
			arg.token = t;
			arg.type = AT_Token;
			return arg;
		}
		arg.reg = r;
		arg.type = AT_Register;
		return arg;
	}
	/* tree */
	p = c.tree;
	if ((p->type == N_Literal) && (p->num_children == 3)) {
		arg.indirect = 1;
		if (p->children[0].type != C_TOKEN) return arg;
		if (ts[p->children[0].index].type != T_OPENP) return arg;
		if (p->children[2].type != C_TOKEN) return arg;
		if (ts[p->children[2].index].type != T_CLOSEP) {
			return arg;
		}
		c = p->children[1];
		if (c.type == C_TOKEN) {
			t = ts[c.index];
			if (t.type != T_WORD) return arg;
			r = reg(t, str);
			if (r < 0) {
				arg.token = t;
				arg.type = AT_Token;
				return arg;
			}
			arg.reg = r;
			arg.type = AT_Register;
			return arg;
		}
		if (c.type == C_VALUE) {
			arg.value = c.value;
			arg.type = AT_Value;
			return arg;
		}
		arg.expr = c.tree;
		arg.type = AT_Expr;
		return arg;
	}
	arg.expr = c.tree;
	arg.type = AT_Expr;
	return arg;
}

static long
icond(char const * name, size_t len)
{
	if (!name) return -1;
	if (len == 0) return 0;
	if ((len == 1) && !strncmpi(name,"c", len)) return  1;
	if ((len == 1) && !strncmpi(name,"n", len)) return  2;
	if ((len == 1) && !strncmpi(name,"v", len)) return  3;
	if ((len == 1) && !strncmpi(name,"z", len)) return  4;
	if ((len == 2) && !strncmpi(name,"ls",len)) return  5;
	if ((len == 2) && !strncmpi(name,"lt",len)) return  6;
	if ((len == 2) && !strncmpi(name,"le",len)) return  7;
	if ((len == 1) && !strncmpi(name,"f", len)) return  8;
	if ((len == 2) && !strncmpi(name,"cc",len)) return  9;
	if ((len == 1) && !strncmpi(name,"p", len)) return 10;
	if ((len == 2) && !strncmpi(name,"nv",len)) return 11;
	if ((len == 2) && !strncmpi(name,"nz",len)) return 12;
	if ((len == 2) && !strncmpi(name,"hi",len)) return 13;
	if ((len == 2) && !strncmpi(name,"ge",len)) return 14;
	if ((len == 2) && !strncmpi(name,"gt",len)) return 15;
	return -1;
}

static int
opfind(struct Node *ins, struct Token *ts, char const * str,
       long *cond, int *last)
{
	struct Arg arg;
	struct InsMeta im;
	struct Token t;
	char const *s;
	size_t const n = nargs(ins);
	size_t i;
	size_t j;
	if (!ins || !ts || !str || !last) return -1;
	*last = -1;
	i = op(ins);
	if (i < 0) return -1;
	t = ts[i];
	s = str + t.start;
	for (i = 0; i < sizeof(instrs)/sizeof(instrs[0]); ++i) {
		im = instrs[i];
		if (strncmpi(s,im.name,strlen(im.name)) != 0) continue;
		*cond = icond(s + strlen(im.name),
		             t.length - strlen(im.name));
		if (*cond < 0) continue;
		if ((im.narg < 0) && (*cond != 0)) continue;
		*last = i;
		if (im.narg < 0) {
			/* direct data, check all immediate */
			for (j = 0; j < n; ++j) {
				arg = get_arg(ins, j, ts, str);
				if (arg.indirect) break;
				if (arg.type == AT_Register) break;
			}
			if (j >= n) return i;
		}
		if (n != (size_t)(im.narg)) continue;
		if (n >= 1) {
			arg = get_arg(ins, 0, ts, str);
			if (!(addr_mode(arg) & im.op1mode)) continue;
		}
		if (n >= 2) {
			arg = get_arg(ins, 1, ts, str);
			if (!(addr_mode(arg) & im.op2mode)) continue;
		}
		return i;
	}
	return -1;
}

static int
reg(struct Token t, char const * str)
{
	char c;
	if (t.type != T_WORD) return -1;
	if (t.length != 2) return -1;
	if (tolower(str[t.start]) == 's'
	    && tolower(str[t.start + 1]) == 'p') {
		return 7;
	}
	if (tolower(str[t.start]) != 'r') return -1;
	c = str[t.start + 1];
	if (isdigit(c) && (digittoint(c) < 8)) return digittoint(c);
	return -1;
}

static int
op(struct Node *n) {
	int i;
	if (!n) return -1;
	for (i = n->num_children - 1; i >= 0; --i) {
		if (n->children[i].type == C_TOKEN) {
			return n->children[i].index;
		}
	}
	return -1;
}

static int
do_instr(struct Node *ins, struct DynArr *istream, size_t *offset,
         int * section, struct Token *ts, char const * str)
{
	struct Token t;
	struct InsMeta im;
	struct Instruction x;
	struct Instruction temp;
	unsigned long encoding;
	long cond = 0;
	int i;
	int j;
	int n;
	int last = -1;
	if (!ins || !istream || !offset || !ts || !str) return 0;
	n = nargs(ins);
	i = opfind(ins, ts, str, &cond, &last);
	if (i < 0) {
		i = op(ins);
		t = ts[i];
		print_err();
		if (last < 0) {
			fprintf(stderr,
			        "unknown instruction \"%.*s\" "
			        "at line %zu, column %zu\n",
			        (int)(t.length), str + t.start,
			        t.line, t.column);
			perr_locus(t, str);
			return 0;
		}
		fprintf(stderr,
		        "invalid address mode for \"%.*s\" "
		        "at line %zu, column %zu\n",
		        (int)(t.length), str + t.start,
		        t.line, t.column);
		perr_locus(t,str);
		return 0;
	}
	im = instrs[i];
	if (im.narg == -2) {
		*section = 1;
		*offset = 0;
		return 1;
	}
	if (im.narg < 0) {
		j = 0;
		while (j < n) {
			if (im.shortable) {
				x = mkbyte(get_arg(ins, j, ts, str));
			} else {
				x = mkword(get_arg(ins, j, ts, str));
			}
			if (*section == 0) push_instr(istream, x);
			if (!istream->valid) return 0;
			++j;
		}
		*offset += im.shortable ? n : 2*n;
		return 1;
	}
	encoding =  ((unsigned long)(im.opcode)) << 11;
	encoding |= ((unsigned long)(cond)) << 7;
	encoding |= ((unsigned long)(im.force_mode)) << 6;
	x.short_type = im.short_type;
	switch (n) {
	case 0:
		x.source.type = AT_None;
		x.filled = 1;
		x.length = 2;
		x.encoding[0] = (encoding>>8)&0xFF;
		x.encoding[1] =  encoding    &0xFF;
		x.encoding[2] = x.encoding[3] = 0;
		if (*section == 0) push_instr(istream, x);
		*offset += x.length;
		break;
	case 1:
		x.source = get_arg(ins, 0, ts, str);
		x.encoding[2] = x.encoding[3] = 0;
		if (x.source.type == AT_Register) {
			x.filled = 1;
			x.length = 2;
			if (im.needs_dest) {
				encoding |= x.source.reg << 3;
			} else {
				encoding |= x.source.reg;
			}
		} else if (x.source.type == AT_Value) {
			x.filled = 1;
			encoding |= (1UL<<6);
			if (fits(x.source.value, im.short_type)) {
				x.length = 2;
				encoding |= x.source.value & 7;
			} else {
				x.length = 4;
				encoding |= ext(im.short_type);
				temp = mkword(x.source);
				x.encoding[2] = temp.encoding[0];
				x.encoding[3] = temp.encoding[1];
			}
		} else {
			x.filled = 0;
			x.length = 4;
			encoding |= (1UL<<6);
		}
		x.encoding[0] = (encoding>>8)&0xFF;
		x.encoding[1] =  encoding    &0xFF;
		if (*section == 0) push_instr(istream, x);
		*offset += x.length;
		break;
	case 2:
		x.source = get_arg(ins, 1, ts, str); /* source */
		x.encoding[2] = x.encoding[3] = 0;
		if (x.source.type == AT_Register) {
			x.filled = 1;
			x.length = 2;
			encoding |= (unsigned long)(x.source.reg);
		} else if (x.source.type == AT_Value) {
			x.filled = 1;
			encoding |= (1UL<<6);
			if (im.short_type != S_S3DE
			    && fits(x.source.value, im.short_type)) {
				x.length = 2;
				encoding |= x.source.value & 7;
			} else {
				x.length = 4;
				encoding |= ext(im.short_type);
				temp = mkword(x.source);
				x.encoding[2] = temp.encoding[0];
				x.encoding[3] = temp.encoding[1];
			}
		} else {
			x.filled = 0;
			x.length = 4;
			encoding |= (1UL<<6);
			encoding |= ext(im.short_type);
		}
		encoding |= get_arg(ins, 0, ts, str).reg << 3;
		x.encoding[0] = (encoding>>8)&0xFF;
		x.encoding[1] =  encoding    &0xFF;
		if (*section == 0) push_instr(istream, x);
		*offset += x.length;
		break;
	default:
		break;
	}
	return istream->valid;
}

static struct Instruction
mkbyte(struct Arg arg)
{
	struct Instruction x;
	x.source = arg;
	if (x.source.type == AT_Value) {
		x.filled = 1;
		x.encoding[0] = (unsigned char)(x.source.value);
		if (x.source.value > 255 || x.source.value < -128) {
			print_warn();
			fprintf(stderr,
			        "value %ld does not fit in a byte, "
			        "truncating\n",
			        x.source.value);
		}
	} else {
		x.filled = 0;
		x.encoding[0] = 0;
	}
	x.length = 1;
	x.short_type = S_NONE;
	return x;
}

static struct Instruction
mkword(struct Arg arg)
{
	struct Instruction x;
	x.source = arg;
	unsigned long v;
	if (x.source.type == AT_Value) {
		v = (unsigned long)(x.source.value);
		x.filled = 1;
		x.encoding[0] = (unsigned char)(v % 256);
		x.encoding[1] = (unsigned char)((v / 256) % 256);
		if (x.source.value > 65535 || x.source.value < -32768) {
			print_warn();
			fprintf(stderr,
			        "value %ld does not fit in a byte, "
			        "truncating\n",
			        x.source.value);
		}
	} else {
		x.filled = 0;
		x.encoding[0] = 0;
		x.encoding[1] = 0;
	}
	x.length = 2;
	x.short_type = S_NONE;
	return x;
}

static int
fits(long value, enum ShortType type)
{
	value &= 0xFFFF;
	if (value & (1L<<15)) {
		value ^= 0xFFFF;
		value += 1;
		value &= 0xFFFF;
		value = -value;
	}
	switch (type) {
	case S_S3:
		return ((value > -4) && (value < 4));
	case S_U3:
		return ((value > 0) && (value < 8));
	case S_S3DE:
		return ((value > -8) && (value < 8) && ((value&1) == 0));
	default:
		break;
	}
	return 0;
}

static unsigned long
ext(enum ShortType type)
{
	if (type == S_S3 || type == S_S3DE) return 4;
	return 0;
}

static int
eval(struct Node *expr, struct DynArr labels,
     struct Token *ts, char const * str, long *value)
{
	struct Label *ls = labels.contents;
	struct Token t;
	struct Token x;
	struct Label lblL;
	struct Label lblR;
	size_t lbli;
	long left;
	long right;
	int needlr = 1;
	switch (expr->type) {
	case N_Expr:
		if (expr->num_children != 3) break;
		if (expr->children[1].type != C_TOKEN) break;
		t = ts[expr->children[1].index];
		if (t.value == '-') {
			if (expr->children[0].type == C_TOKEN
			    && expr->children[2].type == C_TOKEN) {
				x = ts[expr->children[0].index];
				lbli = lookup(labels, x, str);
				if (lbli >= labels.size) break;
				lblL = ls[lbli];
				left = lblL.offset;
				x = ts[expr->children[2].index];
				lbli = lookup(labels, x, str);
				if (lbli >= labels.size) break;
				lblR = ls[lbli];
				right = lblR.offset;
				if (lblL.section != lblR.section) break;
				needlr = 0;
			}
		}
		if (needlr) {
			if (!get_child(expr->children[0], labels,
			               ts, str, &left)) {
				break;
			}
			if (!get_child(expr->children[2], labels,
			               ts, str, &right)) {
				break;
			}
		}
		switch (t.value) {
		case '+':
			*value = left + right;
			return 1;
		case '-':
			*value = left - right;
			return 1;
		case '|':
			*value = left | right;
			return 1;
		case '^':
			*value = left ^ right;
			return 1;
		default:
			break;
		}
		break;
	default:
		break;
	}
	return 0;
}

static int
get_child(struct Child c, struct DynArr labels,
          struct Token *ts, char const * str, long *value)
{
	if (!value) return 0;
	if (c.type == C_VALUE) {
		*value = c.value;
		return 1;
	}
	if (c.type != C_TREE) return 0;
	return eval(c.tree, labels, ts, str, value);
}

static int
strncmpi(char const *s1, char const *s2, size_t n)
{
	size_t i;
	char c1;
	char c2;
	for (i = 0; i < n; ++i) {
		c1 = tolower(s1[i]);
		c2 = tolower(s2[i]);
		if (!c1 &&  c2) return -1;
		if ( c1 && !c2) return  1;
		if (!c1 && !c2) return 0;
		if (c1 < c2) return -1;
		if (c2 < c1) return  1;
	}
	return 0;
}
