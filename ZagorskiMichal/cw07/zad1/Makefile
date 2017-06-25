CC=gcc
SRCS=consumer.c producer.c master.c
OBJS=consumer.o producer.o master.o
EXECS=consumer producer master
CFLAGS=-Wall -pedantic -O0 -Wextra -std=c99
#CFLAGS+=-g

all: compile

full: clean compile

compile: $(EXECS)

% : %.o
	$(CC) $(CFLAGS)  $< -o $@

%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY: clean clean compile all full
clean:
	@rm -f $(EXECS) $(OBJS)
	@echo "Cleaning done"