CC=gcc
CFLAGS=-Wall -pedantic -std=c99
EXEC = readfile generator
OBJ = readfile.o generator.o
SRC = readfile.c generator.c
FILE = file
R_SIZE = 4 512 1024 2048 4096 8192
R_COUNT = 1000 2000 5000 10000 100

all: clean $(EXEC)
	@echo "Build finished succesfully"
	
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

#$(EXEC): $(OBJ)
% : %.o
	$(CC) $(CFLAGS) $< -o $@ 
run: $(R_SIZE) $(R_COUNT) 
	@echo "Running generator"
	# @ for O in R_COUNT{
	# 	for
	# }
	# @cp $(FILE) $(FILE)_lib
	# @cp $(FILE) $(FILE)_sys


.PHONY: clean all run
clean:
	@echo "Cleaning project"
	@rm -f *.o
	@rm -f $(EXEC)
