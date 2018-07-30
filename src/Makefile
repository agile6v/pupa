.SUFFIXES: .c .o .lo

CC=gcc
CFLAGS=-Wall -g -fPIC -D_GNU_SOURCE

UNAME := $(shell uname)
ifeq ($(UNAME), Linux)
	CFLAGS += -D_PUPA_LINUX
else ifeq ($(UNAME), Darwin)
	CFLAGS += -D_PUPA_DARWIN
endif

LIB_PATH=/usr/local/lib
INC_PATH=/usr/local/include/pupa

PUPA_STATIC_OBJS = pupa.o pupa_store.o pupa_shm.o
PUPA_SHARED_OBJS = pupa_store.lo pupa_shm.lo pupa.lo
HEADER_FILES = pupa.h pupa_config.h pupa_store.h pupa_shm.h

ALL_OBJS = $(PUPA_STATIC_OBJS) $(PUPA_SHARED_OBJS)

SHARED_LIBS = libpupa.so
STATIC_LIBS = libpupa.a
ALL_LIBS = $(SHARED_LIBS) $(STATIC_LIBS)

all: $(ALL_OBJS) $(ALL_LIBS)

$(SHARED_LIBS):
	$(CC) $(CFLAGS) -o $@ $< -shared $(PUPA_SHARED_OBJS)

$(STATIC_LIBS):
	ar rcs $@ $(PUPA_STATIC_OBJS)

.o:
	$(CC) $(CFLAGS) -o $@ $<  $(PUPA_STATIC_OBJS)
.c:
	$(CC) $(CFLAGS) -o $@ $<  $(PUPA_STATIC_OBJS)
.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<
.c.lo:
	$(CC) $(CFLAGS) -c -o $@ $<

install:
	mkdir -p $(LIB_PATH)
	mkdir -p $(INC_PATH)

	install -m 755 $(SHARED_LIBS) $(LIB_PATH)
	install -m 644 $(HEADER_FILES) $(INC_PATH)
	
uninstall:
	rm -f $(LIB_PATH)/$(SHARED_LIBS)
	rm -rf $(INC_PATH)

clean:
	rm -f *.o *.so *.lo *.a

.PHONY: install clean
