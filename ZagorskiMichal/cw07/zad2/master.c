#define _XOPEN_SOURCE 700
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
/*#include <sys/ipc.h>
#include <sys/shm.h>*/
#include <errno.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <wait.h>
#include <string.h>

#define handle(fun, val, text, exited)                                   \
                        if(fun val){                                     \
                            fprintf(stderr, "Line %d: ", __LINE__);      \
                            perror(text "\n");                           \
                            if(exited){                                  \
                                exit(EXIT_FAILURE);                      \
                            }                                            \
                            errno = 0;                                   \
                        }

#define SIZE 1024 

int mem_fd = -1;
int* mem = NULL;
sem_t *semaphore = NULL;
int full_report = 1;

void sig_handler(int signo){
    if(signo == SIGINT){
        exit(EXIT_SUCCESS);
    }
}

void at_exit(void){
    handle(sem_close(semaphore), == -1, "Can't close semaphore", 1);
    handle(sem_unlink("/sem1"), == -1, "can't unlink semaphore", 1);
    if(mem_fd > 0){
        close(mem_fd);
    }
    handle(shm_unlink("/mem"), == -1, "Can't unlink memory", 1);
    handle(munmap(mem, SIZE), ==-1, "Can't remove shared memory segment",1);
}

void reader(int number){
    int found = 0;
    struct timeval r_time;

    for(int i = 0; i < SIZE; i++){
        handle(sem_wait(semaphore), == -1, "Reader: Can't drop semaphore", 1);
        if(mem[i] == number){
            gettimeofday(&r_time, NULL);
            found++;
            if(full_report){
                printf("(%d %ld:%ld) Znalazłem %d liczb o wartości %d\n", 
                    getpid(), r_time.tv_sec, r_time.tv_usec, found, number);
            }
        }
        handle(sem_post(semaphore), == -1, "Reader: Can't post semaphore", 1);
    }    
    gettimeofday(&r_time, NULL);
    printf("(%d %ld:%ld) Znalazłem %d liczb o wartości %d\n", 
        getpid(), r_time.tv_sec, r_time.tv_usec, found, number);
    _exit(EXIT_SUCCESS);
}

void writer(){
    int counter = rand();
    int begin = rand() % SIZE;
    struct timeval w_time;
    while(counter--){
        int index = rand() % SIZE;
        handle(sem_wait(semaphore), == -1, "Writer: Can't drop semaphore", 1);
        handle(0, == 0, "Writer: Debug", 0);
        gettimeofday(&w_time, NULL);
        mem[index] = begin;
        printf("(%d %ld:%ld) Wpisałem liczbę %d na pozycję %d. Pozostało %d zadań\n", 
            getpid(), w_time.tv_sec, w_time.tv_usec, begin, index, counter);
        begin++;
        handle(sem_post(semaphore), == -1, "Writer: Can't post semaphore", 1);
        if(begin < begin-1) break;
    }
    _exit(EXIT_SUCCESS);
}

int main(int argc, char** argv){
/*    handle(argc, < 5,  "Too few arguments\n"
        "There should be [-u]", 1);*/
    handle(argc, > 2,  "Too many arguments\n"
        "There should be [-u]", 1);

    if(argc == 6){
        int cmp = strncmp(argv[5], "-u", 3);
        if(cmp == 0){
            full_report = 0;
        }
    }
    handle((semaphore = sem_open("/sem1", O_RDWR, 0666, 1)), == SEM_FAILED, "Can't create semaphore", 1);
    handle((mem_fd = shm_open("/mem", O_RDWR | O_CREAT, 0666)), == -1, "Can't set shared memory object", 1);
    handle(ftruncate(mem_fd, SIZE*(sizeof(int))), == -1, "Can't set memory size", 1);
    handle((mem = (int*)mmap(NULL, SIZE*(sizeof(int)), PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, 0)), == NULL, "can't create Shared memory segment", 1);
    
    for(int i = 0; i < SIZE; i++){
        mem[i] = 0;
    }
    int writers = rand() % (SIZE*5);
    int readers = rand() % (SIZE*5);
    for(int i = 0; i < writers; i++){
        pid_t wr = fork();
        if(wr == 0){
    //        handle(0, == 0, "Debug", 0);
            writer();
            _exit(EXIT_SUCCESS);
        }
        if(i < readers){
            pid_t rd = fork();
            if(rd == 0){
                reader( rand() % SIZE );
            }
        }
    }
   // handle(0, == 0, "Debug", 0);
    for(int i = writers; i < readers; i++){
        pid_t rd = fork();
        if(rd == 0){
            reader( rand() % SIZE );
        }
    }
   // handle(0, == 0, "Debug", 0);
    atexit(&at_exit);
    for(int i = 0; i < readers + writers; i++){
        int status;
        handle( wait(&status), == -1, "Can't wait for children", 0);
    }
    exit(EXIT_SUCCESS);
}