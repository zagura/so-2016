CC=gcc
CFLAGS=-Wall -pedantic -std=c99
EXEC =locks.exe
OBJ =locks.o
SRC =locks.c
FILE =file
CFLAGS += -g
all: compile
#@echo "Build finished succesfully"

run: $(EXEC)
	@echo "Running generator and test programs"
	./run.sh > readfile.log

compile: $(EXEC)
	
%.exe : %.o
	$(CC) $(CFLAGS) $< -o $@
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

#.PHONY: clean all run
.PHONY: all
clean:
	@echo "Cleaning project"
	@rm -rf files
	@rm -f $(EXEC)
	@echo "Cleaning done"

clean_res: clean
	@echo "Cleaning results"
	@rm -f wyniki.txt readfile.log
	@echo "Cleaning done"
