#define _GNU_SOURCE
#define _XOPEN_SOURCE 700
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>

#define handle(fun, val, text, exited)                                   \
                        if(fun val){                                     \
                            fprintf(stderr, "Line %d: ", __LINE__);      \
                            perror(text "\n");                           \
                            if(exited){                                  \
                                exit(EXIT_FAILURE);                      \
                            }                                            \
                            errno = 0;                                   \
                        }

#define SIZE 20
#define SHM 3
#define SEM 4
#define KEY_SIZE 21
#define DEC_BASE 10                        
//key size as max amount of chars for representing unsigned long long int with null byte
int mem = -1;
int semaphores = -1;
int* shmem = NULL;

void sig_INT(int signo){
    if(signo == SIGINT){
        exit(EXIT_SUCCESS);
    }
}
void at_exit(void){
    handle(shmdt(shmem), == -1, "Can't deatach shared memory segment", 0);
}
int main(int argc, char** argv){
    srand(time(NULL));

    handle(argc, != 3, "Consumer: wrong number of arguments\n"
        "There should be <key for memory> <key for semaphore>", 1);
    key_t shm_key = atoi(argv[1]);
    key_t sem_key = atoi(argv[1]);
    handle((mem = shmget(shm_key, 0, IPC_CREAT | 0666)), 
        == -1, "Can't get shared memory", 1);
    handle((semaphores = semget(sem_key, SIZE + 3, IPC_CREAT | 0666)), == -1, "Can't get semaphores set", 1);
    handle((shmem = (int*)shmat(mem, shmem, 0)), == (void*)-1, "Can't attach shered memory", 1);
    handle(atexit(&at_exit), == -1, "Can't set exit action",1);
    signal(SIGINT, &sig_INT);
    struct timeval mz_time;
    //handle(0, ==0, "Consumer: Debbug", 0);
    while(1){
        struct sembuf access;
        access.sem_num = SIZE+2;
        access.sem_op = -1;
        access.sem_flg = SEM_UNDO;
        struct sembuf read;
        read.sem_num = SIZE+0;
        read.sem_op = -1;
        read.sem_flg = SEM_UNDO;
        struct sembuf op;
        op.sem_op = -1;
        op.sem_flg = SEM_UNDO;
        handle(semop(semaphores, &access, 1), == -1, "Can't drop main semaphore", 0);
        handle(semop(semaphores, &read, 1), == -1, "Can't drop index semaphore", 0);
        int index = shmem[SIZE+0];
        shmem[SIZE+0] = (index+1)% SIZE;
        op.sem_num = index;
        handle(semop(semaphores, &op, 1), == -1, "Can't drop element's semaphore", 0);
        int waiting = SIZE;
        waiting -= semctl(semaphores, SIZE+2, GETVAL);
        handle(waiting, >SIZE, "Can't get semaphore value", 0);
        gettimeofday(&mz_time, NULL);
        int val = shmem[index];

        switch(val%2){
            case 0:{
                printf("(%d %ld:%ld) Sprawdziłem liczbę %d na pozycji %d -  liczba parzysta. Liczba zadan oczekujacych %d\n",
                   getpid(), mz_time.tv_sec, mz_time.tv_usec / 1000, val, index, waiting);
                break;
            }
            case 1:{
                printf("(%d %ld:%ld) Sprawdziłem liczbę %d na pozycji %d -  liczba nieparzysta. Liczba zadan oczekujacych %d\n",
                   getpid(), mz_time.tv_sec, mz_time.tv_usec / 1000, val, index, waiting);
                break;
            }
            default:{
                handle(!, NULL, "Undefined", 0);
            }
        }
        op.sem_num = index;
        op.sem_op = 1;
        op.sem_flg = 0;
        handle(semop(semaphores, &op, 1), == -1, "Can't up element's semaphore", 0);
        read.sem_op = 1;
        read.sem_flg = 0;
        read.sem_num = SIZE + 1;
        handle(semop(semaphores, &read, 1), == -1, "Can't up index semaphore", 0);
        access.sem_num = SIZE + 2;
        access.sem_op = 1;
        access.sem_flg = 0;
        handle(semop(semaphores, &access, 1), == -1, "Can't up main semaphore", 0);
    }
    return 0;
}