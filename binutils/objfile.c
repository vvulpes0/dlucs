#include <stdio.h>
#include <stdlib.h>
#include "dlucsas.h"

static struct DynArr gext(struct Object, struct Token *, char const *);
static void gextE(struct DynArr, struct Node *,
                  struct Token *, char const *,
                  struct DynArr *);
static void gextT(struct DynArr, struct Token,
                  char const *, struct DynArr *);
static void warg(FILE *, struct Arg,
                 struct DynArr, struct DynArr,
                 struct Token *, char const *);
static void wchild(FILE *, struct Child,
                   struct DynArr, struct DynArr,
                   struct Token *, char const *);
static void wexpr(FILE *, struct Node *,
                  struct DynArr, struct DynArr,
                  struct Token *, char const *);
static void wtoken(FILE *, struct Token,
                   struct DynArr, struct DynArr,
                   char const *);
static void wvalue(FILE *, long);

/* object file format:
 * magic: DLv1
 * list of needed external labels
 *  - format is two-byte little-endian count, then for each label:
 *    - byte length
 *    - then name
 * list of labels
 *  - format is two-byte little-endian count, then for each label:
 *    - one-byte section
 *    - two-byte little-endian offset
 *    - then byte length
 *    - then name
 * ram size: two-byte little-endian
 * instruction stream length (two-bytes, little-endian)
 * instruction stream
 * list of relocation entries
 *  - two-byte little-endian count, then for each fixup point:
 *    - two-byte little-endian offset
 *    - type:
 *      - 'B' = 8-bit direct value
 *      - 'S' = signed 3-bit
 *      - 'U' = unsigned 3-bit
 *      - 'D' = value-(addr+1) as signed 3-bit, doubled, even [for OFFS]
 *      - 'I' = 16-bit direct value
 *      - 'X' = value-(addr+2) [for OFFS]
 *    - expression in RPN notation
 *      - number is '#' then two-byte little-endian data
 *      - external label is '?' then two-byte little-endian index
 *      - internal label is '@' then two-byte little-endian index
 *      - other operations are by ascii:
 *        - sum '+'
 *        - difference '-'
 *        - or '|'
 *        - xor '^'
 *        - product '*'
 *        - and '&'
 *        - shift-left '<'
 *        - shift-right '>'
 *        - quotient '/'
 *        - negate '_'
 *        - complement '~'
 *    - the character '!'
 */

void
write_object(FILE *file, struct Object obj,
             struct Token *ts, char const * str)
{
	struct DynArr exts = gext(obj, ts, str);
	struct DynArr inns = obj.labels;
	struct DynArr inst = obj.istream;
	struct Instruction *xs = inst.contents;
	struct Label *ls = exts.contents;
	size_t i;
	size_t j;
	size_t length;
	size_t nrelocs = 0;
	size_t offset = 0;
	size_t v;
	char type;
	fprintf(file, "DLv1%c%c",
	        (int)(exts.size&0xFF), (int)((exts.size>>8)&0xFF));
	for (i = 0; exts.valid && i < exts.size; ++i) {
		length = ls[i].length;
		if (length > 255) length = 255;
		fprintf(file, "%c", (int)(length));
		fprintf(file, "%.*s", (int)(length), str + ls[i].start);
	}
	ls = inns.contents;
	fprintf(file, "%c%c",
	        (int)(inns.size&0xFF),
	        (int)((inns.size>>8)&0xFF));
	for (i = 0; inns.valid && i < inns.size; ++i) {
		length = ls[i].length;
		if (length > 255) length = 255;
		fprintf(file, "%c", (int)(ls[i].section&0xFF));
		fprintf(file, "%c%c",
		        (int)(ls[i].offset&0xFF),
		        (int)((ls[i].offset>>8)&0xFF));
		fprintf(file, "%c", (int)(length));
		fprintf(file, "%.*s", (int)(length), str + ls[i].start);
	}
	fprintf(file, "%c%c",
	        (int)(obj.ram_size&0xFF),
	        (int)((obj.ram_size>>8)&0xFF));
	length = 0;
	for (i = 0; inst.valid && i < inst.size; ++i) {
		length += xs[i].length;
	}
	fprintf(file, "%c%c",
	        (int)(length&0xFF), (int)((length>>8)&0xFF));
	for (i = 0; inst.valid && i < inst.size; ++i) {
		for (j = 0; j < xs[i].length; ++j) {
			fprintf(file, "%c", xs[i].encoding[j]);
		}
	}
	nrelocs = 0;
	for (i = 0; inst.valid && i < inst.size; ++i) {
		nrelocs += !xs[i].filled;
	}
	fprintf(file, "%c%c",
	        (int)(nrelocs&0xFF), (int)((nrelocs>>8)&0xFF));
	offset = 0;
	for (i = 0; inst.valid && i < inst.size; ++i) {
		offset += xs[i].length;
		if (xs[i].filled) continue;
		if (xs[i].length == 1) {
			v = offset - 1;
			type = 'B';
		} else if (xs[i].length == 2) {
			v = offset - 1;
			type = "SUDI"[xs[i].short_type];
			if (type == 'I') v = offset - 2;
		} else {
			v = offset - 2;
			type = "IIXI"[xs[i].short_type];
		}
		fprintf(file, "%c%c%c",
		        (int)(v&0xFF), (int)((v>>8)&0xFF), type);
		warg(file, xs[i].source, exts, inns, ts, str);
	}
	free(exts.contents);
}

static struct DynArr
gext(struct Object obj, struct Token *ts, char const * str)
{
	struct Instruction *xs = obj.istream.contents;
	struct Instruction x;
	struct DynArr out;
	size_t i;
	out.size = 0;
	out.valid = 0;
	out.capacity = 4096;
	out.contents = malloc(out.capacity * sizeof(struct Label));
	if (!out.contents) return out;
	out.valid = 1;
	for (i = 0; i < obj.istream.size; ++i) {
		x = xs[i];
		if (x.filled) continue;
		if (x.source.type == AT_Token) {
			gextT(obj.labels, x.source.token, str, &out);
			continue;
		}
		if (x.source.type == AT_Expr) {
			gextE(obj.labels, x.source.expr, ts, str, &out);
		}
		if (!out.valid) return out;
	}
	return out;
}

static void
gextE(struct DynArr labels, struct Node *n, struct Token *ts,
      char const * str, struct DynArr *out)
{
	size_t i;
	if (!n || !ts || !str || !out) return;
	for (i = 0; i < n->num_children; ++i) {
		if (n->children[i].type == C_TOKEN) {
			gextT(labels,ts[n->children[i].index],str, out);
			continue;
		}
		if (n->children[i].type != C_TREE) continue;
		gextE(labels, n->children[i].tree, ts, str, out);
	}
}

static void
gextT(struct DynArr labels, struct Token t,
      char const * str, struct DynArr *out)
{
	struct Label label;
	size_t lbli;
	if (!str || !out) return;
	if (t.type != T_WORD) return;
	lbli = lookup(labels, t, str);
	if (lbli < labels.size) return;
	lbli = lookup(*out, t, str);
	if (lbli < out->size) return;
	label.start = t.start;
	label.length = t.length;
	label.line = t.line;
	label.offset = 0;
	push_label(out, label);
}

static void
warg(FILE *file, struct Arg arg,
     struct DynArr exts, struct DynArr inns,
     struct Token *ts, char const * str)
{
	if (arg.type == AT_Value) {
		wvalue(file, arg.value);
	} else if (arg.type == AT_Token) {
		wtoken(file, arg.token, exts, inns, str);
	} else if (arg.type == AT_Expr) {
		wexpr(file, arg.expr, exts, inns, ts, str);
	}
	fprintf(file, "!");
}
static void
wvalue(FILE *file, long v)
{
	if (!file) return;
	fprintf(file, "#%c%c", (int)(v&0xFF), (int)((v>>8)&0xFF));
}
static void
wtoken(FILE *file, struct Token t,
       struct DynArr exts, struct DynArr inns,
       char const * str)
{
	size_t lbli;
	if (!file || !str) return;
	lbli = lookup(exts, t, str);
	if (lbli < exts.size) {
		fprintf(file, "?");
	} else {
		lbli = lookup(inns, t, str);
		fprintf(file, "@");
	}
	fprintf(file, "%c%c", (int)(lbli&0xFF), (int)((lbli>>8)&0xFF));
}
static void
wexpr(FILE *file, struct Node *expr,
      struct DynArr exts, struct DynArr inns,
      struct Token *ts, char const * str)
{
	struct Token t;
	if (!file || !expr) return;
	switch (expr->type) {
	case N_Expr:
		t = ts[expr->children[1].index];
		wchild(file,expr->children[0],exts,inns,ts,str);
		wchild(file,expr->children[2],exts,inns,ts,str);
		fprintf(file, "%c", (int)(t.value));
		break;
	case N_Term:
		if (expr->num_children == 2) {
			t = ts[expr->children[0].index];
			wchild(file,expr->children[1],exts,inns,ts,str);
			if (t.value == '~') {
				fprintf(file, "~");
			} else if (t.value == '-') {
				fprintf(file, "_");
			}
			break;
		}
		t = ts[expr->children[0].index];
		wchild(file,expr->children[0],exts,inns,ts,str);
		wchild(file,expr->children[2],exts,inns,ts,str);
		fprintf(file, "%c", (int)(t.value));
		break;
	case N_Literal:
		if (expr->num_children == 3) {
			wchild(file,expr->children[1],exts,inns,ts,str);
		} else if (expr->num_children == 1) {
			wchild(file,expr->children[0],exts,inns,ts,str);
		}
		break;
	default:
		break;
	}
}
static void
wchild(FILE *file, struct Child c,
       struct DynArr exts, struct DynArr inns,
       struct Token *ts, char const * str)
{
	if (!file) return;
	if (c.type == C_TOKEN) {
		wtoken(file, ts[c.index], exts, inns, str);
	} else if (c.type == C_VALUE) {
		wvalue(file, c.value);
	} else {
		wexpr(file, c.tree, exts, inns, ts, str);
	}
}
