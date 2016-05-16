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



#define NAME_SIZE 128

sem_t** semaphores = NULL;
pthread_t* threads = NULL;
pthread_mutex_t common_mutex = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t start_mutex = PTHREAD_MUTEX_INITIALIZER;
int capacity = 0;
int k = 0;
int on_board = 0;
pthread_cond_t land_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t start_cond = PTHREAD_COND_INITIALIZER;
int landing_queue = 0;
int starting_queue = 0;
pid_t pid = 0;
int planes = 0;
struct arg{
	int id;
};

typedef struct arg arg_t;

void INT_handler(int signo){
	printf("Catched signal: %d\n", signo);
	if(signo == SIGINT){
		for(int i = 0; i < planes; i++){
			handle((errno = pthread_cancel(threads[i])), != 0, "Can't cancel thread", 0);
		}
	}
	pthread_exit(NULL);
}


void exit_handler(void){
	handle((errno = pthread_cond_destroy(&land_cond)), != 0, "Can't destroy land condition", 0);
	handle((errno = pthread_cond_destroy(&start_cond)), != 0, "Can't destroy start condition", 0);
	handle((errno = pthread_mutex_destroy(&common_mutex)), != 0, "Can't common mutex.", 0);
	if(threads != NULL){
		free(threads);
		threads = NULL;
	}
}

void* run_plane(void* args){
	srand(pthread_self());
	handle(args, == NULL, "Bad argument of thread", 0);
	int id = ((arg_t*)args)->id;

	for(;;){
		fprintf(stdout, "%d\t: Flying...\n", id);
		sleep(rand() % 2 + 1);

		/*
		 *	Landing stage
		 */
		handle((errno = pthread_mutex_lock(&common_mutex)), != 0, "Can't lock common_mutex before landing", 0);
		while((on_board > k && starting_queue > 0) || on_board == capacity){
			landing_queue++;
			handle((errno = pthread_cond_wait(&land_cond, &common_mutex)), != 0, "Can't wait on landing condition", 0);
			landing_queue--;
		}
		on_board++;
		fprintf(stdout, "%d\t: Landing... --- On board: %d\n", id, on_board);
		handle((errno = pthread_cond_broadcast(&start_cond)), != 0, "Can't signal after landing", 0);
		handle((errno = pthread_mutex_unlock(&common_mutex)), != 0, "Can't unlock common_mutex after landing", 0);
		
		fprintf(stdout, "%d\t: Waiting...\n", id);
		sleep(rand() % 2 + 1);

		/*
		 *	Starting stage
		 */
		handle((errno = pthread_mutex_lock(&common_mutex)), != 0, "Can't lock common_mutex before starting", 0);
		while((on_board < k && landing_queue > 0)){
			starting_queue++;
			handle((errno = pthread_cond_wait(&start_cond, &common_mutex)), != 0, "Can't wait on starting condition", 0);
			starting_queue--;
		}
		on_board--;
		fprintf(stdout, "%d\t: Starting... --- on Board %d \n", id, on_board);
		handle((errno = pthread_cond_broadcast(&land_cond)), != 0, "Can't signal after starting", 0);
		handle((errno = pthread_mutex_unlock(&common_mutex)), != 0, "Can't after common_mutex before starting", 0);
	}
	return NULL;
}




int main(int argc, char** argv){
	handle(argc, != 4,"Wrong amount of arguments <ship capacity> <total planes> <k - prioroty switch value>", 0);
	capacity = atoi(argv[1]);
	planes = atoi(argv[2]);
	k = atoi(argv[3]);

	threads = (pthread_t*)calloc(planes, sizeof(pthread_t));

	sigset_t set;
	handle(sigfillset(&set), == -1, "Can't create signal set", 1);
	handle(sigdelset(&set, SIGINT), == -1, "Can't add SIGINT to mask", 1);
	handle(sigprocmask(SIG_SETMASK, &set, NULL), == -1, "Can't set sigmask", 1);

	for(int i = 0; i < planes; i++){
		arg_t args;
		args.id = i+1;
		handle((errno = pthread_create(&threads[i], NULL, &run_plane, &args)), != 0, "Can't swap new thread.", 0);
		sleep(1);
	}

	signal(SIGINT, &INT_handler);

	for(int i = 0; i < planes; i++){
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