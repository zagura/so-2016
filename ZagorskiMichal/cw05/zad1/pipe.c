#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define handle(fun, val, text, exited) if(fun val){										\
							fprintf(stderr, "Line %d", __LINE__);		\
							perror(text "\n"); 											\
							if(exited){												\
								exit(EXIT_FAILURE); 								\
							}														\
						}

int main(int argc, char** argv){
	handle(argc, != 2, "Wrong number of arguments", 1)
	//int n = atoi(argv[1]);
	int descriptors[2];
	handle(pipe(descriptors), == -1, "Creating pipe", 1)
	pid_t cpid = fork();
	handle(cpid, == -1, "Error when forking", 1)
	if(cpid == 0){
		handle(dup2(descriptors[0], 1), == -1, "Can't create pipe from stdout", 1)
		handle(execlp("/usr/bin/tr", "tr", "'[:lower:]'", "'[:upper:]'", NULL), == -1, "tr exec", 1)
		handle(close(descriptors[0]), == -1, "Can't close pipe-out file", 1)

	}
	else if(cpid > 0){
		int status;
		handle(waitpid(cpid, &status,1), == -1, "Error in waitng for child process", 1)
		handle(dup2(descriptors[1], 0), == -1, "Can't create pipe from stdin", 1)
		handle(execlp("/usr/bin/fold", "fold" , "-w", argv[1], NULL), ==-1, "fold exec", -1)
		handle(close(descriptors[1]), == -1, "Can't close pipe-in file", 1)
		//execlp("fold", "-w", argv[1], )
	}
	return 0;
}