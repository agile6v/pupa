.SUFFIXES: .c .o

CC=gcc
CFLAGS=-std=c99 -Wall -g

INC_PATH = -I/usr/local/include/pupa
LIB_PATH = -lpupa

ALL_PRGS = pupa_tool

all: $(ALL_PRGS)

.c:
	$(CC) $(CFLAGS) -o $@ $< $(LIB_PATH) $(INC_PATH)
.c.o:
	$(CC) $(CFLAGS) -c -o $@ $< $(INC_PATH)
clean:
	rm -f $(ALL_PRGS) pupa.store

.PHONY: clean
