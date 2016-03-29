#define _POSIX_C_SOURCE 200801L

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>

int counter = 0;
int usr2 = 1;
void count(int signo){
	if(signo == SIGUSR1){
		counter++;
//		printf("SIGUSR1: %d\n", counter);
	}
}

void printer(int signo){
	if(signo == SIGUSR2){
		usr2 = 0;
		printf("Catched SIGUSR2\n");
	}
}

int main(int argc, char** argv){
	sigset_t old;
	sigset_t set;
	int signal_error = 0;
	//signal_error += sigemptyset(&old);
	//signal_error += sigfillset(&set);
	signal_error += sigfillset(&set);
	//signal_error += sigdelset(&set, SIGUSR2);
	signal_error += sigdelset(&set, SIGUSR1);
	signal_error += sigprocmask(SIG_BLOCK, &set, &old);
	if(signal_error < 0){
		perror("SIGMASK:\n");
		return EXIT_FAILURE;
	}
	sigset_t suspend;
	sigfillset(&suspend);
	sigdelset(&suspend, SIGUSR1);
	sigdelset(&suspend, SIGUSR2);
	struct sigaction act;
	act.sa_handler = count;
	act.sa_mask = set;
	act.sa_flags = 0;
	struct sigaction old_act;
	sigaction(SIGUSR1, &act, &old_act);
	signal(SIGUSR2, &printer);
	printf("Parent: %s\n", argv[1]);
	if(errno != 0) perror(NULL);
	printf("Catcher 43\n");
	pid_t parent = getppid();
	if(argc == 2){
		parent = (pid_t)atoi(argv[1]);
	}
	printf("Catcher 48: %d\n", parent);
	while(usr2){
		sigsuspend(&suspend);
		fprintf(stderr, "Catched counter: %d --- %d\n", counter, usr2);
	}
	while(counter--){
		kill(parent, SIGUSR1);
	}
	kill(parent, SIGUSR2);
	printf("Catcher 57\n");
	return 0;
}