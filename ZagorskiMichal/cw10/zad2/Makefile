CC=gcc
SRCS=client.c server.c
OBJS=client.o server.o
EXECS=client server
CFLAGS=-Wall -pedantic -O0 -Wextra -std=c11
LDFLAGS=-pthread
DEBUG=

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