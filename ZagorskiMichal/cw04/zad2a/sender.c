#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

int counter = 0;
int usr2 = 1;
void count(int signo){
	counter++;
}
void printer(int signo){
	usr2 = 0;
	printf("Received SIGUSR2\n");
}

int main(int argc, char** argv){
	if(argc != 2){
		fprintf(stderr, "Wrong number of arguments!\n");
		return EXIT_FAILURE;
	}
	int sends = atoi(argv[1]);
	if(sends <= 0){
		fprintf(stderr, "To low number in parameter\n");
		return EXIT_FAILURE;
	}
	sigset_t set, old;
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
	char catcher[] = "./catcher";
	int send_counter = sends;
	char arg[10];
	fprintf(stderr, "Forking\n");
	pid_t pid = fork();
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
	if(pid == 0){
		//execlp(catcher, "catcher", (char*)NULL);
		fprintf(stderr, "Child process\n");
		sprintf(arg, "%d\n", getppid());
		execlp(catcher, "catcher", arg, (char*)NULL);
		fprintf(stderr, "Finished catching\n");
		_exit(0);
	}
	if(pid == -1){
		perror("Fork error\n");
	}
	if(pid > 0){
		fprintf(stderr, "Parent proces\n");
		sleep(1);
		while(send_counter--){
			kill(pid, SIGUSR1);
		}
		kill(pid, SIGUSR2);

		int status = 0;
		while(usr2){
			sigsuspend(&suspend);
		//	pause();
		}
		printf("Received %d SIGUSR1 signals\n", counter);
		printf("Signals send: %d\n", sends);
		fprintf(stderr, "Wating for child process\n");
		if( waitpid(pid, &status, 0) != pid){
			perror("Wait:\n");
		}
		if(WIFSIGNALED(status)){
			printf("Catcher exit by signal %d.\n", WTERMSIG(status));
			return EXIT_FAILURE;
		}
	}
	return 0;
}