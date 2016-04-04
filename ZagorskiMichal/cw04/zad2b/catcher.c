#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>

int counter = 0;
int usr2 = 1;
pid_t parent;
void count(int signo){
	if(signo == SIGUSR1){
		counter++;
	}
	kill(parent, SIGUSR1);
}

void printer(int signo){
	if(signo == SIGUSR2){
		usr2 = 0;
		printf("Child:\n Catched SIGUSR2\nReceived SIGUSR1: %d\n----------\n", 
			counter);
	}
}

int main(int argc, char** argv){
	sigset_t old;
	sigset_t set;
	int signal_error = 0;

	signal_error += sigfillset(&set);
	signal_error += sigprocmask(SIG_BLOCK, &set, &old);

	if(signal_error < 0){
		perror("SIGMASK:\n");
		return EXIT_FAILURE;
	}

	sigset_t suspend;
	signal_error += sigfillset(&suspend);
	signal_error += sigdelset(&suspend, SIGUSR1);
	signal_error += sigdelset(&suspend, SIGUSR2);

	if(signal_error < 0){
		perror("SIGMASK:\n");
		return EXIT_FAILURE;
	}

	struct sigaction act;
	act.sa_handler = count;
	act.sa_mask = set;
	act.sa_flags = 0;
	struct sigaction old_act;

	sigaction(SIGUSR1, &act, &old_act);
	signal(SIGUSR2, &printer);

	if(errno != 0) perror(NULL);
	parent = getppid();

	if(argc == 2){
		parent = (pid_t)atoi(argv[1]);
	}

	while(usr2){
		sigsuspend(&suspend);
		if(usr2){

		}
	}

	kill(parent, SIGUSR2);
	return 0;
}