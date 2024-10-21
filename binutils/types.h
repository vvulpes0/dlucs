#ifndef TYPES_H
#define TYPES_H
struct DynArr {
	void *contents;
	size_t size;
	size_t capacity;
	_Bool valid;
};

enum TokenState {
	ST_BEGIN,
	ST_BIN,
	ST_COMMENT,
	ST_DEC,
	ST_HEX,
	ST_WORD,
};

enum TokenType {
	T_WORD,
	T_NUM,
	T_OP,
	T_COLON,
	T_COMMA,
	T_NL,
	T_OPENP,
	T_CLOSEP,
	T_END,
};

struct Token {
	enum TokenType type;
	size_t start;
	size_t length;
	size_t line;
	size_t column;
	long value;
};

enum NodeType {
	N_Prog,
	N_Instruction,
	N_ArgList,
	N_Expr,
	N_Term,
	N_Literal,
};

struct Node {
	int type;
	struct Child *children;
	size_t num_children;
};

enum ChildType {
	C_TOKEN,
	C_TREE,
	C_VALUE,
};

struct Child {
	int type;
	union {
		struct Node *tree;
		size_t index;
		long value;
	};
};

struct Object {
	struct DynArr istream;
	struct DynArr labels;
	size_t ram_size;
};

union NC { struct Node n; struct Child c; };

enum ArgType {
	AT_Register,
	AT_Value,
	AT_Token,
	AT_Expr,
	AT_None,
};
struct Arg {
	int type;
	_Bool indirect;
	union {
		int reg;
		long value;
		struct Token token;
		struct Node * expr;
	};
};
struct Instruction {
	struct Arg source;
	size_t length;
	int filled;
	int short_type;
	unsigned char encoding[4];
};

struct Label {
	size_t start;
	size_t length;
	size_t line;
	size_t offset;
	int section;
};

struct Symbol {
	char name[256];
	size_t offset;
	int section;
	unsigned char size;
};
#endif
