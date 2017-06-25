#define _GNU_SOURCE
#define _XOPEN_SOURCE 700
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include <string.h>

#define RECORD_SIZE 1024
#define VARIANT 1

#define handle(fun, val, text, exited)                                   \
                        if(fun val){                                     \
                            fprintf(stderr, "Line %d: ", __LINE__);      \
                            perror(text "\n");                           \
                            if(exited){                                  \
                                exit(EXIT_FAILURE);                      \
                            }                                            \
                            errno = 0;                                   \
                        }

pthread_t* threads = NULL;
pthread_mutex_t mutex;
int fd = 0;
int thread_count = 0;

struct rec{
    int id;
    char text[RECORD_SIZE - sizeof(int)];
};

typedef struct rec record_t;
record_t* content = NULL;
char word[RECORD_SIZE];

struct arg{
    int id;
};

typedef struct arg arg_t;

int records = 0;
void* run(void* arg){

    arg_t args = *(arg_t*)arg;
    handle(arg, == NULL, "Wrong thread arg", 1);
    int id = args.id;
    record_t* content = calloc(records, sizeof(record_t));
    ssize_t bytes = -1;
    int old_cancel = 0;

    switch(VARIANT){
        case 1:
        case 2:
            pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &old_cancel);
            break;
        case 3:
            pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &old_cancel);
            break;
    }
    handle(errno, != 0, "Set cancel state", 0);

    switch(VARIANT){
        case 1:
            errno = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &old_cancel);
            break;
        case 2:
            errno = pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &old_cancel);
            break;
    }
    handle(errno, != 0, "Set cancel type", 0);

    for(int index = 0; index < records && bytes != 0; index++){
        handle((errno = pthread_mutex_lock(&mutex)), != 0, "Cannot lock mutex", 0);
        bytes = read(fd, &content[index], RECORD_SIZE);
        handle(bytes, == -1, "Error while reading file", 0);
        handle((errno = pthread_mutex_unlock(&mutex)), != 0, "Cannot unlock mutex", 0);
        if(VARIANT == 2){
            pthread_testcancel();
        }
        for(unsigned int i = 0; i < (RECORD_SIZE - sizeof(int) - strlen(word) + 1); i++){
            if(content[index].text[i] == ' ' || i == 0){
                int cmp = 0;
                cmp = strncmp(&content[index].text[i+1], word, strlen(word) - 1);
                if(cmp == 0){
                    fprintf(stdout, "Thread: %lu, word %s in record number: %d\n", pthread_self(), word, content[index].id);
                    if(VARIANT < 3){
                        for(int otid = 0; otid < thread_count; otid++){
                            if(otid != id){
                                pthread_cancel(threads[otid]);
                            }
                        }
                        if(content != NULL){
                            free(content);
                        }
                        return NULL;
                    }
                }
            }
        }
        usleep(2000);
    }
    if(content != NULL){
        free(content);
    }
    return NULL;
}

void at_exit(void){
    if(threads != NULL){
        free(threads);
    }
    handle(close(fd), == -1, "Can't close file", 1);
    handle((errno = pthread_mutex_destroy(&mutex)), != 0, "Cannot destroy mutex", 0);
}


int main(int argc, char** argv){
    handle(argc, != 5, "Wrong number of arguments, should be <threads count> "
        "<filename> <records count> <searched word>", 1);
    fd = open(argv[2], O_RDONLY);
    handle(fd, == -1, "Can't open given file", 1);
    thread_count = atoi(argv[1]);
    records = atoi(argv[3]);
    strcpy(word, argv[4]);    
    threads = (pthread_t*)calloc(thread_count, sizeof(pthread_t));
    arg_t* tdata = (arg_t*)calloc(thread_count, sizeof(arg_t));
    atexit(&at_exit);
    handle((errno = pthread_mutex_init(&mutex, NULL)), != 0, "Cannot init mutex", 0);
    for(int tid = 0; tid < thread_count; tid++){
        tdata[tid].id = tid;
        handle((errno = pthread_create(&threads[tid], NULL, &run, &tdata[tid])), != 0, "Can't create thread", 0);
    }
    if(tdata != NULL){
        free(tdata);
    }
    for(int tid = 0; tid < thread_count; tid++){
        void* ret_val;
        switch(VARIANT){
            case 1: case 2:
                handle((errno = pthread_join(threads[tid], &ret_val)), != 0, "Can't join thread", 0);
                break;
            case 3:
            handle((errno = pthread_detach(threads[tid])), != 0, "Can't detach thread", 0);
                break;
       }
    }

    return EXIT_SUCCESS;    
}