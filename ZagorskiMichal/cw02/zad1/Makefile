CC=gcc
CFLAGS=-Wall -pedantic -std=c99
EXEC =readfile.exe generator.exe
OBJ =readfile.o generator.o
SRC =readfile.c generator.c
FILE =file
CFLAGS += -O3
all: clean $(EXEC)
	@echo "Build finished succesfully"

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