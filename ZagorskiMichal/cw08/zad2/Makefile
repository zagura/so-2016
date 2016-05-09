CC=gcc
SRCS=zad2a.c
OBJS=zad2a.o
EXECS=zad2a
CFLAGS=-Wall -pedantic -O0 -Wextra -std=c11
LDFLAGS=-pthread
DEBUG=

run: full
	./$(EXECS) 9 Textfile.txt 10 Russia-Serbia

all: compile

full: clean compile

compile: $(EXECS)

% : %.o
	$(CC) $(CFLAGS) $(DEBUG) $< -o $@ $(LDFLAGS)

.c.o:
	$(CC) -c $(CFLAGS) $(DEBUG) $< -o $@ $(LDFLAGS)

.PHONY: clean clean compile all full
clean:
	@rm -f $(EXECS) $(OBJS)
	@echo "Cleaning done"

$(EXECS):$(OBJS)
