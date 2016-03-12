CC=gcc
CFLAGS=-Wall -pedantic -std=c99
EXEC = readfile generator
OBJ = readfile.o generator.o
SRC = readfile.c generator.c


all: $(EXEC)

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) $< $@

.PHONY: clean all
clean:
	@rm -f *.o
	@rm -f $(EXEC)
