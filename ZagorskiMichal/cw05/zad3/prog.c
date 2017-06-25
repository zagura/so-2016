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


#define handle(fun, val, text, exited)                              \
                    if(fun val){                                    \
                        fprintf(stderr, "Line %d\n", __LINE__);     \
                        perror(text "\n");                          \
                            if(exited){                             \
                                exit(EXIT_FAILURE);                 \
                            }                                       \
                        errno = 0;                                  \
                    }

int main(int argc, char** argv){

    handle(argc, != 2, "Wrong number of arguments", 1)

    FILE* p_file = popen("ls -l | grep ^d", "r");
    handle(p_file, == NULL, "Bad popen", 1)

    FILE* output = fopen(argv[1], "w");
    handle(output, == NULL, "Bad open output file", 0)

    char* line = NULL;
    size_t size = 0;

    while(getline(&line, &size, p_file) != -1){
        fprintf(output, "%s", line);
    }

    free(line);
    handle(pclose(p_file), == -1, "Error, when closing pipe", 1)
    handle(fclose(output), == -1, "Error, when closing output file", 1)

    return EXIT_SUCCESS;
}
