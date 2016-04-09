#define _GNU_SOURCE
#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define handle(fun, val, text, exited) if(fun val){										\
							fprintf(stderr, "Line %d\n", __LINE__);						\
							perror(text "\n"); 											\
							if(exited){													\
								exit(EXIT_FAILURE); 									\
							}															\
						}

int main(int argc, char** argv){
	handle(argc, != 2, "Wrong number of arguments", 1)

	int descriptors[2];
	handle(pipe(descriptors), == -1, "Creating pipe", 1)

	pid_t cpid = fork();
	handle(cpid, == -1, "Error when forking", 1)

	if(cpid == 0){
		handle(dup2(descriptors[0], 0), == -1, "Can't create pipe from stdin", 1)
		handle(close(descriptors[0]), == -1, "Can't close pipe-in file", 1)
		execl("/usr/bin/fold", "fold" , "-w", argv[1], NULL);
		return EXIT_FAILURE;
	}
	else if(cpid > 0){
		handle(dup2(descriptors[1], 1), == -1, "Can't create pipe from stdout", 1)
		handle(close(descriptors[1]), == -1, "Can't close pipe-out file", 1)
		execl("/usr/bin/tr", "tr", "'[:lower:]'", "'[:upper:]'", NULL);
		return EXIT_FAILURE;
	}
	return 0;
}