#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "dlucsas.h"

static void collapseNL(struct DynArr *);
static char * read_file(FILE *);
static void usage(FILE *);
static void version(FILE *);

int
main(int argc, char **argv)
{
	char *arg;
	char const *outfile = NULL;
	char const *infile = NULL;
	char *str;
	struct DynArr ts;
	struct Node *node;
	struct Object out;
	int retval = 0;
	int i;
	int should_help = 0;
	int should_version = 0;

	i = 1;
	while (argv[i]) {
		arg = argv[i];
		if ((strncmp("-",arg,1) != 0) || (strlen(arg) == 1)) {
			break;
		}
		++arg;
		while (arg && *arg) {
			if (*arg == 'h') {
				++arg;
				should_help = 1;
			} else if (*arg == 'v') {
				++arg;
				should_version = 1;
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
	if (argv[i]) infile = argv[i];

	if (should_version) version(stdout);
	if (should_help) usage(retval ? stderr : stdout);
	if (should_help || should_version) return retval;

	if (infile) {
		stdin = freopen(infile, "rb", stdin);
		if (!stdin) {
			perror("dlucsas");
			return 1;
		}
	}
	if (!outfile) outfile = "a.out";
	stdout = freopen(outfile, "wb", stdout);
	if (!stdout) {
		fclose(stdin);
		perror("dlucsas");
		return 1;
	}

	str = read_file(stdin);
	ts = tokenize(str);
	if (!ts.valid) {
		free(str);
		free(ts.contents);
		fclose(stdin);
		fclose(stdout);
		return 1;
	}
	collapseNL(&ts);
	node = parse(ts, str);
	node = simplify(node, ts, 0).tree;
	out = emit(node, ts.contents, str);
	while (relax(out, ts.contents, str));
	flush(out, ts.contents, str);
	if (!out.istream.valid || !out.labels.valid) {
		retval = 1;
	} else {
		/* display_tree(node,ts,str); */
		write_object(stdout, out, ts.contents, str);
	}
	fclose(stdin);
	fclose(stdout);

	destroy_obj(&out);
	free_tree(node);
	free(str);
	free(ts.contents);
	return retval;
}

static void
collapseNL(struct DynArr *ts)
{
	struct Token * cs;
	size_t i = 0;
	size_t j = 0;
	int depth = 0;
	int skipNL = 1;
	enum TokenType type;
	if (!ts || !ts->valid) return;
	cs = ts->contents;
	for (i = 0; i < ts->size; ++i) {
		type = cs[i].type;
		if ((type != T_NL) || ((depth == 0) && !skipNL)) {
			cs[j++] = cs[i];
			skipNL = ((type == T_NL) || (type == T_COLON));
		}
		depth += (type == T_OPENP) - (type == T_CLOSEP);
	}
	ts->size = j;
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
	fprintf(file, "usage: dlucsas [-hv] [-o outfile] [infile]\n");
	fprintf(file, "  -h:         show help and exit\n");
	fprintf(file, "  -o outfile: "
	        "write output to outfile instead of a.out\n");
	fprintf(file, "  -v:         show version number and exit\n");
}

static void
version(FILE *file)
{
	fprintf(file, "dlucsas, version 0.1\n");
}
