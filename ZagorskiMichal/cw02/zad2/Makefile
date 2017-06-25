CC=gcc
CFLAGS=-Wall -pedantic -std=c99
DIR =.
EXEC=dir.exe dir_root.exe
OBJS=dir.o dir_root.exe
SRCS=dir.c dir_root.exe
CFLAGS += -O0
all: compile
#@echo "Build finished succesfully"

# run: $(EXEC)
# 	@echo "Running generator and test programs"
# 	./run.sh > readfile.log

compile: $(EXEC)
	
%.exe : %.o
	$(CC) $(CFLAGS) $< -o $@
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

#.PHONY: clean all run
.PHONY: all
clean:
	@echo "Cleaning project"
	@rm -f $(EXEC)
	@echo "Cleaning done"
