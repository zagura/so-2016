#	Michal Zagorski
#	Systemy Operacyjne 2015/2016
#	6 marca 2016

CC=gcc
CFLAGS=-Wall -pedantic -std=c99
LIBDIR=../lib
BINDIR=../bin
SRCDIR=./
#Project name
NAME=zad2
OBJ=s_test.o
DL_OBJ=dl_test.o

all: clean dir static dynamic
	
dir:
	mkdir $(BINDIR)



static: $(OBJ)
	$(CC) $(CFLAGS) -static -L$(LIBDIR) s_test.o -o s_test -lmz_lib
dynamic: $(OBJ)
	$(CC) $(CFLAGS) -shared -Wl,-soname,libmz_sharedlib.so s_test.o -o d_test
dll: $(DL_OBJ)
	$(CC) $(CFLAGS)  -shared -Wl,-soname,libmz_sharedlib.so dl_test.o -o dl_test
$(OBJ): s_test.c
	$(CC) $(CFLAGS) -c -o $(OBJ) s_test.c -ldl
$(DL_OBJ): dl_test.c
	$(CC) $(CFLAGS) -c -o $(DL_OBJ) dl_test.c

clean:
	rm -f s_test d_test dl_test
	rm -f *.o
	rm -rf ../bin
# static: $(SOBJS) dir
# 	ar crs $(LIBDIR)/$(STATIC) $(SOBJS)

# s_%.o: %.c clean_static
# 	$(CC) $(CFLAGS) -c $< -o $@
# d_%.o: %.c clean_dynamic
# 	$(CC) $(CFLAGS) -fPIC -c $< -o $@

# dynamic: $(DOBJS)
# 	$(CC) $(CFLAGS) -shared -fPIC -o $(LIBDIR)/$(DYNAMIC) $(DOBJS)

# clean_static:
# 	rm -f $(OBJS)
# 	rm -f $(SOBJS)

# clean_dynamic:
# 	rm -f $(DOBJS)

# clean: clean_dynamic clean_static
# 	rm -rf $(LIBDIR)

#-l$(LIBDIR)/mz_list -l$(LIBDIR)/mz_contact