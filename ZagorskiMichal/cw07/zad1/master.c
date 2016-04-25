//#define _GNU_SOURCE
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
#include <wait.h>

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
#define KEY_SIZE 12
#define DEC_BASE 10                        
//key size as max amount of chars for representing int with null byte
int mem = -1;
int semaphores = -1;
void sig_handler(int signo){
    if(signo == SIGINT){
      //  struct 
        handle(shmctl(mem, IPC_RMID, NULL), == -1, "Can't remove shm segment", 1);
        exit(EXIT_SUCCESS);
    }
}

int main(int argc, char** argv){
    handle(argc, != 4, "Wrong number of arguments\n"
        "There should be <consumer count> <producer count> <file for shared memory>", 1);
    int mem = -1;
    key_t shm_key;
    handle((shm_key = ftok(argv[3], SHM)), == -1, "Can't get key for shared memory", 1);
    handle((mem = shmget(shm_key, (SIZE + 2)*sizeof(int), IPC_CREAT | 0666)), 
        == -1, "Can't get shared memory", 1);
    // SIZE for elements
    // index == size+1 for read indexer
    // index == size for write indexer
    key_t sem_key;
    handle((sem_key = ftok(argv[3], SEM)), == -1, "Can't get key for semaphore", 1);
    semaphores = -1;
    handle((semaphores = semget(sem_key, SIZE + 3, IPC_CREAT | 0666)), == -1, "Can't get semaphores set", 1);
    // SIZE of content table,
    // 1 for queue
    // 1 for consumer indexer and 1 for producer indexer
    int consumer = 0;
    signal(SIGINT, &sig_handler);
    int index = 0;
    unsigned short* sem_vals = (unsigned short* )calloc(SIZE+3, sizeof(unsigned short));
    for(index = 0; index < SIZE + 2; index++){
        sem_vals[index] = 1;
    }
    sem_vals[index] = SIZE;
 //   union semun sem_begin;
  //  sem_begin.array = sem_vals;
    handle(semctl(semaphores, 0, SETALL, sem_vals), == -1, "Can't set semaphores begin state", 1);
    if(sem_vals != NULL){
        free(sem_vals);
    }
    int* shmem = NULL;
    handle((shmem = (int*)shmat(mem, shmem, 0)), == (void*)-1, "Can't attach shered memory", 1);
    for(int i = 0; i < SIZE + 2; i++){
        shmem[i] = 0;
    }
    handle(shmdt(shmem), == -1, "Can't deatach shared memory segment", 0);


    int clients = atoi(argv[1]) + atoi(argv[2]);
    char key1[KEY_SIZE];
    char key2[KEY_SIZE];
    sprintf(key1, "%d", shm_key);
    sprintf(key2, "%d", sem_key);
 //   handle(0, == 0, "Debbug", 0);
    int producer;
    for(producer = 0; producer < atoi(argv[2]); producer++){
        pid_t child = fork();
        if(child == 0){
            sleep(1);
            execl("./producer", "producer", key1 , key2, NULL);
            perror("Error in executing producer");
        }
        handle(child, == -1, "Can't create producer process", 0);
    }
    for(consumer = 0; consumer < atoi(argv[1]); consumer++){
        pid_t child = fork();
        if(child == 0){
            sleep(1);
            execlp("./consumer", "consumer", key1 , key2, NULL);
            perror("Error in executing consumer");
        }
        handle(child, == -1, "Can't create consumer process", 0);
    }
    int all = 0;
    for(all = 0; all < clients; all++){
        int status;
        handle(wait(&status), == -1, "Can't get child exit status", 0);
    }
    sig_handler(SIGINT);
    //pause();
    return 0;
}