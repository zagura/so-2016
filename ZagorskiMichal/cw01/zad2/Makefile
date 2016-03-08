#	Michal Zagorski
#	Systemy Operacyjne 2015/2016
#	6 marca 2016

CC=gcc
CFLAGS=-Wall -std=c99
#-pedantic
LIBDIR=../lib
BINDIR=../bin
#../bin
SRCDIR=./
#Project name
NAME=zad2
OBJ=s_test.o
DL_OBJ=dl_test.o

all: clean dir static dynamic dll
#export LD_LIBRARY_PATH=../
dir:
	mkdir $(BINDIR)



static: $(OBJ)
	$(CC) $(CFLAGS) -L$(LIBDIR) s_test.o -o $(BINDIR)/s_test -lmz_lib
dynamic: $(OBJ)
	$(CC) $(CFLAGS) s_test.o -L$(LIBDIR) -o $(BINDIR)/d_test -lmz_sharedlib -Wl,-rpath,../lib
dll: $(DL_OBJ)
	$(CC) $(CFLAGS) dl_test.o -L$(LIBDIR) -o $(BINDIR)/dl_test  -ldl
#-Wl,-soname,../lib/libmz_sharedlib.so
$(OBJ): s_test.c
	$(CC) $(CFLAGS) -fPIC -c -o $(OBJ) s_test.c
$(DL_OBJ): dl_test.c
	$(CC) $(CFLAGS) -c -o $(DL_OBJ) -fPIC dl_test.c 


clean:
	rm -f s_test d_test dl_test
	rm -f *.o
	rm -rf $(BINDIR)
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