CC=gcc
SRCS=variant1.c variant2.c variant3.c
OBJS=variant1.o variant2.o variant3.o
EXECS=variant1 variant2 variant3
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
