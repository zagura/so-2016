#define _XOPEN_SOURCE 700
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <errno.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <wait.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#define handle(fun, val, text, exited)                                   \
                        if(fun val){                                     \
                            fprintf(stderr, "Line %d: ", __LINE__);      \
                            perror(text "\n");                           \
                            if(exited){                                  \
                                exit(EXIT_FAILURE);                      \
                            }                                            \
                            errno = 0;                                   \
                        }


#define PHILOSOPHERS 5
#define NAME_SIZE 128

sem_t** semaphores = NULL;
pthread_t* threads = NULL;
pid_t pid = 0;

struct arg{
	int id;
};

typedef struct arg arg_t;

void INT_handler(int signo){
	printf("Catched signal: %d\n", signo);
	if(signo == SIGINT){
		for(int i = 0; i < PHILOSOPHERS; i++){
			handle((errno = pthread_cancel(threads[i])), != 0, "Can't cancel thread", 0);
		}
	}
	pthread_exit(NULL);
}
char* generate_name(int id){
	char* name = (char*)calloc(NAME_SIZE, sizeof(char));
	sprintf(name, "/_%d_%d", getpid(), id);
	return name;
}


void exit_handler(void){
	for(int i = 0; i<= PHILOSOPHERS; i++){
		char* name = generate_name(i);
		handle(sem_close(semaphores[i]), == -1, "Can't close semaphore", 0);
		handle(sem_unlink(name), == -1 , "Can't unlink semaphore", 0);
		if(name != NULL){
			free(name);
			name = NULL;
		}		
	}
	if(semaphores!= NULL){
		free(semaphores);
		semaphores = NULL;
	}


}

void* run(void* args){
	srand(pthread_self());
	handle(args, == NULL, "Bad argument of thread", 0);
	int id = ((arg_t*)args)->id;
	sleep(PHILOSOPHERS - id);
	printf("ID: %d\n\n", id);
	sem_t* left = semaphores[id%PHILOSOPHERS];
	sem_t* right = semaphores[(id+1) % PHILOSOPHERS];
	sem_t* waiter = semaphores[PHILOSOPHERS];
	for(;;){
		handle(sem_wait(waiter), == -1, "Can't drop waiter's sempahore.", 0);
		flockfile(stdout);
		fprintf(stdout, "Waiter accepted request for %d philosopher.\n", id);
		funlockfile(stdout);
		handle(sem_wait(left), == -1, "Can't take left fork.", 0);
		flockfile(stdout);
		fprintf(stdout, "Philosopher %d took left fork.\n", id);
		funlockfile(stdout);
		handle(sem_wait(right), == -1, "Can't take right fork.", 0);
		flockfile(stdout);
		fprintf(stdout, "Philosopher %d took right fork\n", id);
		fprintf(stdout, "%d eating... -------------------------------------\n", id);
		funlockfile(stdout);
		handle(sem_post(right), == -1, "Can't post right fork", 0);
		flockfile(stdout);
		fprintf(stdout, "Philosopher %d returned right fork\n", id);
		funlockfile(stdout);
		handle(sem_post(left), == -1, "Can't post left fork", 0);
		flockfile(stdout);
		fprintf(stdout, "Philosopher %d returned left fork.\n", id);
		funlockfile(stdout);
		handle(sem_post(waiter), == -1, "Can't post waiter's semaphore", 0);
		flockfile(stdout);
		fprintf(stdout, "%d thinking... +++++++++++++++++++++++++++++++++++\n", id);
		funlockfile(stdout);
	}
	return NULL;
}




int main(int argc, char** argv){
	handle(argc, > 1,"To much arguments", 0);
	(void)argv;
	semaphores = (sem_t**)calloc(PHILOSOPHERS+1, sizeof(sem_t*));
	threads = (pthread_t*)calloc(PHILOSOPHERS, sizeof(pthread_t));
	for(int i = 0 ; i < PHILOSOPHERS; i++){
		char* name = generate_name(i);
		handle((semaphores[i] = sem_open(name, O_CREAT, O_EXCL, 0666, 1)), == SEM_FAILED, "Can't create semaphore", 0);
		if(name != NULL){
			free(name);
			name = NULL;
		}		
	}
	char* name = generate_name(PHILOSOPHERS);
	handle((semaphores[PHILOSOPHERS] = sem_open(name, O_CREAT, O_EXCL, 0666, PHILOSOPHERS - 1)), == SEM_FAILED, "Can't create semaphore", 0);
	if(name != NULL){
		free(name);
		name = NULL;
	}
	sigset_t set;
	handle(sigfillset(&set), == -1, "Can't create signal set", 1);
	handle(sigdelset(&set, SIGINT), == -1, "Can't add SIGINT to mask", 1);
	handle(sigprocmask(SIG_SETMASK, &set, NULL), == -1, "Can't set sigmask", 1);

	for(int i = 0; i < PHILOSOPHERS; i++){
		arg_t args;
		args.id = i+1;
		handle((errno = pthread_create(&threads[i], NULL, &run, &args)), != 0, "Can't swap new thread.", 0);
		sleep(1);
	}

	signal(SIGINT, &INT_handler);
	for(int i = 0; i < PHILOSOPHERS; i++){
		pthread_t id = threads[i];
		void* status;
		if(id != 0){
			handle((errno = pthread_join(id, &status)), != 0, "Can't join thread.", 0);
		}
		threads[i] = 0;
	}






	exit_handler();
	return EXIT_SUCCESS;
}