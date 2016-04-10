#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE
#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
//#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <time.h>

#define handle(fun, val, text, exited) 											\
						if(fun val){														\
							fprintf(stderr, "Line %d\n", __LINE__);				\
							perror(text "\n"); 											\
							if(exited){														\
								exit(EXIT_FAILURE); 										\
							}																	\
							errno = 0;														\
						}

FILE* fifo;
char* line = NULL;

void int_handle(int signo){
	fclose(fifo);
	fprintf(stdout, "\nClosing program wrfifo\n");
	if(!line){
		free(line);
	}
	exit(EXIT_SUCCESS);
}


int main(int argc, char** argv){
	handle(argc, != 2, "Wrong number of arguments", 1)
	char* filename = argv[1];
	fifo = fopen(filename, "w");
	signal(SIGINT, &int_handle);
	handle(fifo, == NULL, "Can,t open fifo file", 1)
	
	size_t size = 0;

//	while(1){
		while(getline(&line, &size, stdin) != -1){
			time_t write_time = time(NULL);
			line[strlen(line)-1] = 0;
			fprintf(fifo, "%d - %s - %s", getpid(), line, ctime(&write_time));
			fflush(fifo);
		}
//	}

	int_handle(0);
	return EXIT_SUCCESS;
}
