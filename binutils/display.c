#include <stdio.h>
#include <stdlib.h>

#include "dlucsas.h"

static void
disp(struct Node * root, struct DynArr dts, char const * str, size_t gorn)
{
	struct Token *ts = dts.contents;
	struct Token t;
	size_t i;
	long v;
	if (!root) return;
	printf("%zu [label=\"%c\"]\n", gorn, "PIAETL"[root->type]);
	for (i = 0; i < root->num_children; ++i) {
		printf("%zu -> %zu\n", gorn, 10*gorn + i);
	}
	for (i = 0; i < root->num_children; ++i) {
		if (root->children[i].type == C_TOKEN) {
			t = ts[root->children[i].index];
			printf("%zu [label=\"%.*s\","
			       "style=\"filled\","
			       "fillcolor=\"#ffc\"]\n",
			       10*gorn + i,
			       t.type == T_NL ? 4 : (int)(t.length),
			       t.type == T_NL ? "<NL>" : str + t.start);
		} else if (root->children[i].type == C_TREE) {
			disp(root->children[i].tree,
			     dts, str, 10*gorn + i);
		} else {
			v = root->children[i].value;
			printf("%zu [label=\"%ld\","
			       "style=\"filled\","
			       "fillcolor=\"#8c8\"]\n",
			       10*gorn + i, v);
		}
	}
}

void
display_tree(struct Node * root, struct DynArr dts, char const * str)
{
	printf("digraph {\n");
	disp(root,dts,str,1);
	printf("}\n");
}
