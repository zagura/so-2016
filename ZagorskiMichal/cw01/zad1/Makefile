#	Michal Zagorski
#	Systemy Operacyjne 2015/2016
#	2 marca 2016


CC=gcc
CFLAGS=-Wall -pedantic -std=c99
LIBDIR=../lib
BINDIR=../bin
SRCDIR=./
#Project name
NAME=lib1
OBJS=mz_contact.o mz_list.o
SOBJS=s_mz_contact.o s_mz_list.o
DOBJS=d_mz_contact.o d_mz_list.o
STATIC=libmz_lib.a
DYNAMIC=libmz_sharedlib.so
LIBS=mz_list.h mz_contact.h
SRCS=*.c

all: clean dir static dynamic
	cp $(LIBS) $(LIBDIR)
dir:
	mkdir $(LIBDIR)

static: $(SOBJS) dir
	ar crs $(LIBDIR)/$(STATIC) $(SOBJS)

s_%.o: %.c clean_static
	$(CC) $(CFLAGS) -c $< -o $@
d_%.o: %.c clean_dynamic
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

dynamic: $(DOBJS)
	$(CC) $(CFLAGS) -shared -fPIC  -o $(LIBDIR)/$(DYNAMIC) $(DOBJS)
#-Wl,-soname,$(DYNAMIC)
clean_static:
	rm -f $(OBJS)
	rm -f $(SOBJS)

clean_dynamic:
	rm -f $(DOBJS)

clean: clean_dynamic clean_static
	rm -rf $(LIBDIR)

