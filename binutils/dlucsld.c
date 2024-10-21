#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "dlucsld.h"

static char mem[65536];

static int check_exts(struct DynArr, struct DynArr *,
                      size_t, char const *);
static void do_fixups(struct DynArr, struct DynArr, struct DynArr,
                      size_t, size_t, char const *);
static size_t lookup(struct DynArr, struct Symbol);
static char * read_file(FILE *);
static size_t push_globs(size_t, struct DynArr *,
                         size_t, size_t, char const *);
static size_t push_locs(size_t, struct DynArr *, size_t, size_t,
                        char const *);
static size_t skip_exts(size_t, char const *);
static void usage(FILE *);
static void version(FILE *);

int
main(int argc, char **argv)
{
	struct DynArr exp;
	struct DynArr needs;
	struct DynArr internal;
	char * arg;
	char * str;
	char * outfile;
	size_t load_addr = 0x8000;
	size_t ram_base = 0x0100;
	size_t offset;
	size_t roffset;
	size_t cursor = 0;
	size_t total_size = 0;
	size_t rjump;
	size_t x;
	size_t j;
	int base;
	size_t i = 1;
	int retval = 0;
	int should_help = 0;
	int should_version = 0;
	while (argv[i]) {
		arg = argv[i];
		if ((strncmp("-",arg,1) != 0) || (strlen(arg) == 1)) {
			break;
		}
		++arg;
		while (arg && *arg) {
			if (*arg == 'b') {
				++arg;
				if (!*arg) {
					++i;
					arg = argv[i];
				}
				if (!arg || !*arg) {
					should_help = 1;
					retval = 1;
				} else {
					load_addr = strtol(arg,NULL,0);
				}
				arg = NULL;
			} else if (*arg == 'h') {
				++arg;
				should_help = 1;
			} else if (*arg == 'o') {
				++arg;
				if (!*arg) {
					++i;
					arg = argv[i];
				}
				if (!arg || !*arg) {
					should_help = 1;
					retval = 1;
				} else {
					outfile = arg;
				}
				arg = NULL;
			} else if (*arg == 'r') {
				++arg;
				if (!*arg) {
					++i;
					arg = argv[i];
				}
				if (!arg || !*arg) {
					should_help = 1;
					retval = 1;
				} else {
					ram_base = strtol(arg,NULL,0);
				}
				arg = NULL;
			} else if (*arg == 'v') {
				++arg;
				should_version = 1;
			} else {
				fprintf(stderr,
				        "unrecognized option '-%c'\n",
				        *arg);
				should_help = 1;
				retval = 1;
				++arg;
			}
		}
		++i;
	}

	if (should_help) usage(retval ? stderr : stdout);
	if (should_version) version(stdout);
	if (should_help || should_version) return retval;

	exp.capacity = 4096;
	exp.size = 0;
	exp.valid = 0;
	exp.contents = malloc(exp.capacity * sizeof(struct Symbol));
	if (!exp.contents) { return 1; }
	exp.valid = 1;

	internal.capacity = 4096;
	internal.size = 0;
	internal.valid = 0;
	internal.contents = malloc(internal.capacity
	                           * sizeof(struct Symbol));
	if (!internal.contents) {
		free(exp.contents);
		return 1;
	}
	internal.valid = 1;

	needs.capacity = 4096;
	needs.size = 0;
	needs.valid = 0;
	needs.contents = malloc(needs.capacity
	                           * sizeof(struct Symbol));
	if (!needs.contents) {
		free(exp.contents);
		return 1;
	}
	needs.valid = 1;

	if (!argv[i]) {
		fprintf(stderr, "dlucsld: no input files!\n");
		return 1;
	}
	offset = load_addr;
	roffset = ram_base;
	base = i;
	while (argv[i]) {
		cursor = 0;
		stdin = freopen(argv[i],"rb",stdin);
		if (!stdin) {
			perror("dlucsld");
			free(exp.contents);
			return 1;
		}
		str = read_file(stdin);
		fclose(stdin);
		if (!str) {
			free(exp.contents);
			return 1;
		}

		cursor = 4;
		cursor = skip_exts(cursor, str);
		cursor = push_globs(cursor, &exp, offset, roffset, str);
		/* bump ram position */
		x = (unsigned char)(str[cursor++]);
		x |= ((unsigned char)(str[cursor++]))<<8;
		roffset += x;
		/* fill instructions */
		x = (unsigned char)(str[cursor++]);
		x |= ((unsigned char)(str[cursor++]))<<8;
		for (j = 0; j < x; ++j) {
			mem[offset++] = str[cursor++];
		}

		free(str);
		++i;
	}

	offset = load_addr;
	roffset = ram_base;
	i = base;
	while (argv[i]) {
		cursor = 0;
		stdin = freopen(argv[i],"rb",stdin);
		if (!stdin) {
			perror("dlucsld");
			free(exp.contents);
			return 1;
		}
		str = read_file(stdin);
		fclose(stdin);
		if (!str) {
			free(exp.contents);
			return 1;
		}

		cursor = 4;
		/* read externals: if lacking, skedaddle */
		needs.size = 0;
		if (!check_exts(exp, &needs, cursor, str)) {
			free(exp.contents);
			return 1;
		}
		cursor = skip_exts(cursor, str);
		/* read internals into local buf : 0size'd each iter */
		internal.size = 0;
		cursor = push_locs(cursor,&internal,offset,roffset,str);
		/* get ram offset jump */
		rjump = (unsigned char)(str[cursor++]);
		rjump |= ((unsigned char)(str[cursor++]))<<8;
		/* skip over instruction stream */
		x = (unsigned char)(str[cursor++]);
		x |= ((unsigned char)(str[cursor++]))<<8;
		cursor += x;
		/* apply fixups */
		do_fixups(exp, internal, needs, offset, cursor, str);
		offset += x;
		roffset += rjump;
		++i;
	}

	if (outfile) {
		stdout = freopen(outfile, "w", stdout);
		if (!stdout) {
			perror("dlucsld");
			free(str);
			free(exp.contents);
			free(internal.contents);
			return 1;
		}
	}

	total_size = offset - load_addr;
	cursor = load_addr;
	printf("v3.0 hex words addressed");
	for (i = 0; i < total_size; ++i) {
		if (i%16 == 0) printf("\n%04zx:", i);
		printf(" %02x", (unsigned char)(mem[cursor++]));
	}
	printf("\n");
	fclose(stdout);
	free(str);
	free(exp.contents);
	free(internal.contents);
	return 0;
}

static void
do_fixups(struct DynArr exp, struct DynArr internal, struct DynArr needs,
          size_t offset, size_t i, char const * buf)
{
	struct Symbol *es = exp.contents;
	struct Symbol *is = internal.contents;
	struct Symbol *ns = needs.contents;
	char type;
	size_t j;
	size_t pos;
	long n = (unsigned char)(buf[i++]);
	long a = 0;
	long b = 0;
	n |= ((unsigned char)(buf[i++]))<<8;
	while (n > 0) {
		pos = (unsigned char)(buf[i++]);
		pos |= ((unsigned char)(buf[i++]))<<8;
		pos += offset;
		type = buf[i++];
		while (buf[i] != '!') {
			switch (buf[i]) {
			case '?':
				j = (unsigned char)(buf[++i]);
				j |= ((unsigned char)(buf[++i]))<<8;
				j = ns[j].offset;
				a = b;
				b = es[j].offset;
				break;
			case '@':
				j = (unsigned char)(buf[++i]);
				j |= ((unsigned char)(buf[++i]))<<8;
				a = b;
				b = is[j].offset;
				break;
			case '#':
				j = (unsigned char)(buf[++i]);
				j |= ((unsigned char)(buf[++i]))<<8;
				a = b;
				b = j;
				break;
			case '+':
				b = a + b;
				break;
			case '-':
				b = a - b;
				break;
			case '|':
				b = a | b;
				break;
			case '^':
				b = a ^ b;
				break;
			case '*':
				b = a*b;
				break;
			case '&':
				b = a&b;
				break;
			case '<':
				b = a<<b;
				break;
			case '>':
				b = a>>b;
				break;
			case '/':
				b = a/b;
				break;
			case '_':
				b = -b;
				break;
			case '~':
				b = ~b;
				break;
			}
			++i;
		}
		switch (type) {
		case 'B':
			mem[pos] = b&0xFF;
			break;
		case 'S':
			mem[pos] |= b&7;
			break;
		case 'U':
			mem[pos] |= b&7;
			break;
		case 'D':
			b = b - (pos + 1);
			b /= 2;
			mem[pos] |= b&7;
		case 'I':
			mem[pos] = b&0xFF;
			mem[pos+1] = (b>>8)&0xFF;
			break;
		case 'X':
			b = b - (pos + 2);
			mem[pos] = b&0xFF;
			mem[pos+1] = (b>>8)&0xFF;
			break;
		}
		++i;
		--n;
	}
}

static size_t
lookup(struct DynArr exp, struct Symbol sym)
{
	struct Symbol *xs = exp.contents;
	size_t i;
	for (i = 0; i < exp.size; ++i) {
		if (sym.size != xs[i].size) continue;
		if (strncmp(sym.name, xs[i].name, sym.size)) continue;
		return i;
	}
	return i;
}

static size_t
push_globs(size_t i, struct DynArr *exp, size_t offset, size_t roffset,
           char const * const buf)
{
	struct Symbol sym;
	long n = 0;
	size_t size;
	size_t pos;
	size_t x;
	int section;
	n |= (unsigned char)(buf[i++]);
	n |= ((unsigned char)(buf[i++]))<<8;
	while (n > 0) {
		section = (unsigned char)(buf[i++]);
		pos = (unsigned char)(buf[i++]);
		pos |= (unsigned char)(buf[i++])<<8;
		pos += section ? roffset : offset;
		size = (unsigned char)(buf[i++]);
		if (buf[i] == '_') {
			sym.offset = pos;
			sym.size = size;
			memcpy(sym.name, buf + i, size);
			x = lookup(*exp, sym);
			if (x < exp->size) {
				print_err();
				fprintf(stderr,
				        "multiple definitions of "
				        "symbol \"%.*s\"\n",
				        (int)(size), sym.name);
			}
			push_symbol(exp, sym);
		}
		i += size;
		--n;
	}
	return i;
}

static size_t
push_locs(size_t i, struct DynArr *exp, size_t offset, size_t roffset,
          char const * const buf)
{
	struct Symbol sym;
	long n = 0;
	size_t size;
	size_t pos;
	size_t x;
	int section;
	n |= (unsigned char)(buf[i++]);
	n |= ((unsigned char)(buf[i++]))<<8;
	while (n > 0) {
		section = (unsigned char)(buf[i++]);
		pos = (unsigned char)(buf[i++]);
		pos |= (unsigned char)(buf[i++])<<8;
		pos += section ? roffset : offset;
		size = (unsigned char)(buf[i++]);
		if (buf[i] != '_') {
			sym.offset = pos;
			sym.size = size;
			memcpy(sym.name, buf + i, size);
			x = lookup(*exp, sym);
			if (x < exp->size) {
				print_err();
				fprintf(stderr,
				        "multiple definitions of "
				        "symbol \"%.*s\"\n",
				        (int)(size), sym.name);
			}
			push_symbol(exp, sym);
		}
		i += size;
		--n;
	}
	return i;
}

static char *
read_file(FILE *file)
{
	char *buf;
	char *t;
	size_t capacity = 4096;
	size_t size = 0;
	int c;
	if (!file) return NULL;
	buf = malloc(4096);
	if (!buf) return buf;
	while (((c = fgetc(file))) != EOF) {
		buf[size++] = (char)(c);
		if (size == capacity) {
			capacity *= 2;
			t = realloc(buf, capacity);
			if (!t) {
				free(buf);
				return NULL;
			}
			buf = t;
		}
	}
	buf[size] = '\0';
	return buf;
}

static void
usage(FILE *file)
{
	fprintf(file, "usage: dlucsld [-hv] [-o outfile] infile...\n");
	fprintf(file, "  -b addr:    base ROM load address\n");
	fprintf(file, "  -h:         show help and exit\n");
	fprintf(file, "  -o outfile: "
	        "write output to outfile instead of standard output\n");
	fprintf(file, "  -r addr:    base RAM load address\n");
	fprintf(file, "  -v:         show version number and exit\n");
}

static void
version(FILE *file)
{
	fprintf(file, "dlucsld, version 0.1\n");
}

static int
check_exts(struct DynArr xs, struct DynArr *needs,
           size_t i, char const * buf)
{
	struct Symbol sym;
	size_t size;
	long n = 0;
	n |= (unsigned char)(buf[i++]);
	n |= ((unsigned char)(buf[i++]))<<8;
	while (n > 0) {
		size = buf[i++];
		memcpy(sym.name, buf + i, size);
		sym.size = size;
		sym.offset = lookup(xs, sym);
		if (sym.offset >= xs.size) {
			print_err();
			fprintf(stderr,
			        "undefined symbol \"%.*s\"\n",
			        (int)(size), sym.name);
			return 0;
		}
		push_symbol(needs, sym);
		i += size;
		--n;
	}
	return 1;
}

static size_t
skip_exts(size_t i, char const * buf)
{
	long n = 0;
	n |= (unsigned char)(buf[i++]);
	n |= ((unsigned char)(buf[i++]))<<8;
	while (n > 0) {
		i += (unsigned char)(buf[i]) + 1;
		--n;
	}
	return i;
}
