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
#include <errno.h>
#include <time.h>
#include <signal.h>

FILE* fifo;
char* line = NULL;
#define handle(fun, val, text, exited)                                 \
                        if(fun val){                                   \
                            fprintf(stderr, "Line %d\n", __LINE__);    \
                            perror(text "\n");                         \
                            if(exited){                                \
                                exit(EXIT_FAILURE);                    \
                            }                                          \
                            errno = 0;                                 \
                        }

void int_handle(int signo){
    fclose(fifo);
    fprintf(stdout, "\nClosing program rdfifo\n");
    if(!line){
        free(line);
    }
    exit(EXIT_SUCCESS);
}

int main(int argc, char** argv){
    handle(argc, != 2, "Wrong number of arguments", 1)
    mode_t rights = 666;
    char* filename = argv[1];
    
    handle(mkfifo(filename, rights), == -1 && errno != EEXIST , "Can't create fifo", 1)
    fifo = fopen(filename, "r");
    signal(SIGINT, &int_handle);
    handle(fifo, == NULL, "Can,t open fifo file", 1)
    
    size_t size = 0;

        while(getline(&line, &size, fifo) != -1){
            time_t read_time = time(NULL);
            char* date = ctime(&read_time);
            int index = strlen(date) -1;
            date[index] = 0;
            fprintf(stdout, "%s - %s", date, line);
            date[index] = '\n';
        }

    int_handle(0);
}
