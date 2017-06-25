CC=gcc
CFLAGS=-Wall -pedantic -std=c99
EXEC=printer
OBJS=printer.o
SRCS=printer.c
CFLAGS += -g
all: compile run
#@echo "Build finished succesfully"


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
