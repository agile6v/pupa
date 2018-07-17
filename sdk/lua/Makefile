.SUFFIXES: .c .o .lo

CC=gcc
CFLAGS=

INC_PATH = -I/usr/local/include/pupa
LIB_PATH = -lpupa -llua

PUPA_SHARED_OBJS = pupa_lua.lo
ALL_OBJS = $(PUPA_SHARED_OBJS)

SHARED_LIBS = pupa_lua.so
ALL_LIBS = $(SHARED_LIBS)

all: $(ALL_OBJS) $(ALL_LIBS)

$(SHARED_LIBS):
	$(CC) $(CFLAGS) -o $@ $< -shared -fPIC $(PUPA_SHARED_OBJS) $(INC_PATH) $(LIB_PATH)

.o:
	$(CC) $(CFLAGS) -o $@ $<  $(PUPA_STATIC_OBJS)
.c:
	$(CC) $(CFLAGS) -o $@ $<  $(PUPA_STATIC_OBJS)
.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<  $(INC_PATH) $(LIB_PATH)
.c.lo:
	$(CC) $(CFLAGS) -c -o $@ $<  $(INC_PATH) $(LIB_PATH)

clean:
	rm -f $(SHARED_LIBS) pupa.store

.PHONY: clean
