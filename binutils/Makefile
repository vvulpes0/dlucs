CFLAGS:=$(CFLAGS) -std=c99 -Oz -Wl,-x
CFLAGS:=$(CFLAGS) -Wall -Wno-missing-braces -Wsign-compare
.MAIN : all
.PHONY : all
all : dlucsas dlucsld genucode
dlucsas : dlucsas.c dlucsas.h types.h
dlucsas : display.c err.c simplify.c slr.c tokenize.c
dlucsas : instructions.c instructions.h
dlucsas : dynarr.c objfile.c
	$(CC) $(CFLAGS) -o $(.TARGET) $(.ALLSRC:M*.c)
dlucsld : dlucsld.c dlucsld.h types.h err.c dynarr.c
	$(CC) $(CFLAGS) -o $(.TARGET) $(.ALLSRC:M*.c)
genucode : genucode.c
	$(CC) $(CFLAGS) -o $(.TARGET) $(.ALLSRC:M*.c)
