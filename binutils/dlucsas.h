#ifndef DLUCSAS_H
#define DLUCSAS_H
#include "types.h"
void destroy_obj(struct Object *);
void display_tree(struct Node *, struct DynArr, char const *);
struct Object emit(struct Node *, struct Token *, char const *);
void flush(struct Object, struct Token *, char const *);
void free_tree(struct Node *);
size_t lookup(struct DynArr, struct Token, char const *);
struct Node * parse(struct DynArr, char const *);
void perr_locus(struct Token, char const *);
void print_err(void);
void print_warn(void);
void push_child(struct DynArr *, struct Child);
void push_int(struct DynArr *, int);
void push_instr(struct DynArr *, struct Instruction);
void push_label(struct DynArr *, struct Label);
void push_token(struct DynArr *, struct Token);
int relax(struct Object, struct Token *, char const *);
struct Child simplify(struct Node *, struct DynArr, _Bool);
struct DynArr tokenize(char const *);
void write_object(FILE *, struct Object, struct Token *, char const *);
#endif
