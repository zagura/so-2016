CC=gcc
CFLAGS=-Wall -pedantic -std=c99 -O0
EXEC=prog
OBJS=prog.o
SRCS=prog.c
#CFLAGS += -g
all: compile run


compile: $(EXEC)
	
% : %.o
	$(CC) $(CFLAGS) $< -o $@
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
.PHONY: clean all run
clean:
	@echo "Cleaning project"
	@rm -f $(EXEC)
	@echo "Cleaning done"
