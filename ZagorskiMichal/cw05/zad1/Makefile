CC=gcc
CFLAGS=-Wall -pedantic -std=c99 -O0
EXEC=pipe
OBJS=pipe.o
SRCS=pipe.c
#CFLAGS += -g
all: compile run


compile: $(EXEC)
	
% : %.o
	$(CC) $(CFLAGS) $< -o $@
%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@
.PHONY: clean all run
clean:
	@echo "Cleaning project"
	@rm -f $(EXEC)
	@echo "Cleaning done"
