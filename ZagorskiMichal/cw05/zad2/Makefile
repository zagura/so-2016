CC=gcc
CFLAGS=-Wall -pedantic -std=c99 -O0
EXEC=rdfifo wrfifo
OBJS=rdfifo.o wrfifo.o
SRCS=rdfifo.c wrfifo.c
#CFLAGS += -g
all: compile run


compile: $(EXEC)
	
% : %.o
	$(CC) $(CFLAGS) $< -o $@
%.o: %.c
	echo $@
	$(CC) $(CFLAGS) -c $< -o $@
.PHONY: clean all run
clean:
	@echo "Cleaning project"
	@rm -f $(EXEC)
	@echo "Cleaning done"
