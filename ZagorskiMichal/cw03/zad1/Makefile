CC=gcc
CFLAGS=-Wall -pedantic -std=c99
DIR =.
EXEC=fork vfork clone vclone
OBJS=fork.o vfork.o clone.o vlone.o
SRCS=fork.c vfork.c clone.c vclone.c
CFLAGS += -g
all: compile
#@echo "Build finished succesfully"

# run: $(EXEC)
# 	@echo "Running generator and test programs"
# 	./run.sh > readfile.log

compile: $(EXEC)
	
% : %.o
	$(CC) $(CFLAGS) $< -o $@
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

#.PHONY: clean all run
.PHONY: all
clean:
	@echo "Cleaning project"
	@rm -f $(EXEC)
	@echo "Cleaning done"
