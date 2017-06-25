CC=gcc
SRCS=master.c
OBJS=master.o
EXECS=master
CFLAGS=-Wall -pedantic -O0 -Wextra -std=c99
LFLAGS=-lrt -lpthread
#CFLAGS+=-g

all: compile

full: clean compile

compile: $(EXECS)

$(EXECS) : $(OBJS)
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS)

%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@ $(LFLAGS)

.PHONY: clean clean compile all full
clean:
	@rm -f $(EXECS) $(OBJS)
	@echo "Cleaning done"
