CC=gcc
CFLAGS=-Wall -pedantic -std=c99
EXEC=fcounter
OBJS=fcounter.o
SRCS=fcounter.c
CFLAGS += -g
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
	@rm -f result.txt
