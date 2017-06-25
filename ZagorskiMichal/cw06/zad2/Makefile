CC=gcc
SRCS=server.c client.c
OBJS=server.o client.o
EXECS=server client
CFLAGS=-Wall -pedantic -O0 -Wextra -std=c99
LFLAGS=-lrt
#CFLAGS=-g

all: compile

full: clean compile

compile: $(EXECS)

$(EXECS) : $(OBJS)
	$(CC) $(CFLAGS) server.o -o server -lrt
	$(CC) $(CFLAGS) client.o -o client -lrt

%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@ $(LFLAGS)

.PHONY: clean clean compile all full
clean:
	@rm -f $(EXECS) $(OBJS)
	@echo "Cleaning done"
