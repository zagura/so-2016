CC=gcc
CFLAGS=-Wall -pedantic -std=c99
EXEC = readfile generator
OBJ = readfile.o generator.o
SRC = readfile.c generator.c


all: clean $(EXEC)
	@echo "Build finished succesfully"
	
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) $< -o $@ 

.PHONY: clean all
clean:
	@echo "Cleaning project"
	@rm -f *.o
	@rm -f $(EXEC)
