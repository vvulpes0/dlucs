#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dlucsas.h"

static char const separators[] = "_'_";
static int digitvalue(int);

struct DynArr
tokenize(char const * str)
{
	/* setup and initialization */
	struct DynArr ts;
	struct Token t;
	int state = ST_BEGIN;
	size_t i = 0;
	char c;
	size_t line = 1;
	size_t column = 0;
	size_t old_column = 0;
	ts.capacity = 4096;
	ts.contents = malloc(ts.capacity*sizeof(struct Token));
	ts.size = 0;
	ts.valid = 0;
	if (!ts.contents) {
		ts.capacity = 0;
		return ts;
	}
	ts.valid = 1;
	/* main state machine */
	while (((c = str[i])) != '\0') {
		if (c == '\t') column = 8*(column/8)+7;
		if (!ts.valid) return ts;
		switch (state) {
		case ST_BEGIN:
			if (isspace(c) && c != '\n') break;
			t.start = i;
			t.line = line;
			t.column = column;
			t.length = 1;
			t.value = 0;
			if (c == '$') {
				t.type = T_NUM;
				state = ST_HEX;
			} else if (c == '%') {
				t.type = T_NUM;
				state = ST_BIN;
			} else if (isdigit(c)) {
				t.type = T_NUM;
				state = ST_DEC;
				t.value = digitvalue(c);
			} else if (strchr("*+-/<>^~",c)) {
				t.type = T_OP;
				t.value = c;
				push_token(&ts, t);
			} else if (c == ':') {
				t.type = T_COLON;
				push_token(&ts, t);
			} else if (c == ',') {
				t.type = T_COMMA;
				push_token(&ts, t);
			} else if (c == '\n') {
				t.type = T_NL;
				++line;
				column = -1;
				push_token(&ts, t);
			} else if (c == '(') {
				t.type = T_OPENP;
				push_token(&ts, t);
			} else if (c == ')') {
				t.type = T_CLOSEP;
				push_token(&ts, t);
			} else if (c == ';') {
				t.type = T_NL;
				push_token(&ts, t);
				state = ST_COMMENT;
			} else if (c == '_' || isalpha(c)) {
				t.type = T_WORD;
				state = ST_WORD;
			} else {
				ts.valid = 0;
				fprintf(stderr,
				        "unknown character '%c' "
				        "at line %zu, column %zu\n",
				        c, line, column);
			}
			break;
		case ST_COMMENT:
			if (c == '\n') {
				++line;
				column = -1;
				state = ST_BEGIN;
			}
			break;
		case ST_BIN:
			if (isalnum(c) && !strchr("01",c)) {
				ts.valid = 0;
				fprintf(stderr,
				        "invalid digit in binary "
				        "constant at line %zu, "
				        "column %zu\n",
				        line, column);
				break;
			}
			if (strchr(separators,c)) {
				++t.length;
				break;
			}
			if (!isxdigit(c)) {
				push_token(&ts, t);
				--i;
				column = old_column;
				state = ST_BEGIN;
				break;
			}
			++t.length;
			t.value *= 2;
			t.value += digitvalue(c);
			break;
		case ST_DEC:
			if (isalnum(c) && !isdigit(c)) {
				ts.valid = 0;
				fprintf(stderr,
				        "invalid digit in decimal "
				        "constant at line %zu, "
				        "column %zu\n",
				        line, column);
				break;
			}
			if (strchr(separators,c)) {
				++t.length;
				break;
			}
			if (!isxdigit(c)) {
				push_token(&ts, t);
				--i;
				column = old_column;
				state = ST_BEGIN;
				break;
			}
			++t.length;
			t.value *= 10;
			t.value += digitvalue(c);
			break;
		case ST_HEX:
			if (isalnum(c) && !isxdigit(c)) {
				ts.valid = 0;
				fprintf(stderr,
				        "invalid character in hex "
				        "constant at line %zu, "
				        "column %zu\n",
				        line, column);
				break;
			}
			if (strchr(separators,c)) {
				++t.length;
				break;
			}
			if (!isxdigit(c)) {
				push_token(&ts, t);
				--i;
				column = old_column;
				state = ST_BEGIN;
				break;
			}
			++t.length;
			t.value *= 16;
			t.value += digitvalue(c);
			break;
		case ST_WORD:
			if (!isalnum(c) && c != '_') {
				push_token(&ts, t);
				--i;
				column = old_column;
				state = ST_BEGIN;
				break;
			}
			++t.length;
			break;
		default:
			break;
		}
		++i;
		old_column = column < 0 ? 0 : column;
		++column;
	}
	if (!ts.valid) return ts;
	/* push newline in case file didn't end with one */
	t.type = T_NL;
	t.start = i;
	t.length = 0;
	t.line = line+1;
	t.column = 0;
	t.value = 0;
	push_token(&ts, t);
	/* push end-of-stream sentinel token */
	t.type = T_END;
	push_token(&ts, t);
	return ts;
}

static int
digitvalue(int c)
{
	char const *str = "0123456789abcdef";
	char const *ptr = strchr(str, tolower(c));
	if (!ptr) return 0;
	if (ptr - str > 15) return 0;
	return ptr - str;
}
