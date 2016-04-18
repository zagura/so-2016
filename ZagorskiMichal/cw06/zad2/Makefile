CC=gcc
SRC=server.c client.c
OBJ=server.o client.o
EXEC=server client
CFLAGS=-Wall -pedantic -O0 -Wextra -std=c99
CFLAGS=-g

all: compile

full: clean compile

compile: $(EXEC)

% : %.o
	$(CC) $(CFLAGS) -o $< $@

%.o: %.c
	$(CC) -c $(CFLAGS) $@

.PHONY clean
clean:
	@rm -f $(EXEC) $(OBJ)
	@echo "Cleaning done\n"
