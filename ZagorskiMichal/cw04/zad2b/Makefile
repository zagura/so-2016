CC=gcc
CFLAGS=-Wall -pedantic -std=c99 -O0
EXEC=catcher sender
OBJS=catcher.o sender.o
SRCS=catcher.c sender.c
#CFLAGS += -g
all: compile run
#@echo "Build finished succesfully"

# run: $(EXEC)
# 	@echo "Running generator and test programs"
# 	./run.sh > readfile.log

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
