#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "dlucsas.h"

void
print_err(void)
{
	if (isatty(fileno(stderr))) {
		fprintf(stderr,"\033[31merror:\033[0m ");
	} else {
		fprintf(stderr,"error: ");
	}
}

void
print_warn(void)
{
	if (isatty(fileno(stderr))) {
		fprintf(stderr,"\033[35mwarning:\033[0m ");
	} else {
		fprintf(stderr,"error: ");
	}
}

void
perr_locus(struct Token t, char const * const str)
{
	char *endl;
	int line = t.line;
	int column = t.column;
	int c_line = 1;
	int i = 0;
	int j = 0;
	int width = 0;
	if ((t.type == T_NL) && (t.column == 0)) {
		--line;
		column = -1;
	}
	if (line < 0) return;
	while ((c_line < line) && (str[i] != '\0')) {
		c_line += (str[i++] == '\n');
	}
	if (str[i] == '\0') return;
	endl = strchr(str + i, '\n');
	if (!endl) endl = strchr(str + i, '\0');
	fprintf(stderr, "%.*s\n", (int)(endl - (str + i)), str + i);
	j = 0;
	while (str + i + j < endl) {
		if (str[i+j] == '\t') width = 8*(width/8) + 7;
		++width;
		++j;
	}
	if (column >= 0) width = column;
	for (i = 0; i < width; ++i) {
		fprintf(stderr,"-");
	}
	fprintf(stderr,"^\n");
}
