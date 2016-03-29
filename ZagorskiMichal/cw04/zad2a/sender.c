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
	char catcher[] = "./catcher";
	int send_counter = sends;
	char arg[10];
	fprintf(stderr, "Forking\n");
	pid_t pid = fork();
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
		sleep(2);
		while(send_counter--){
			kill(pid, SIGUSR1);
			sleep(1);
		}
		signal(SIGUSR2, &printer);
		signal(SIGUSR1, &count);
		int status = 0;
		fprintf(stderr, "Wating for child process\n");
		if( wait(&status) != pid){
			perror("Wait:\n");
		}
		if(WIFSIGNALED(status)){
			printf("Catcher exit by signal %d.\n", WTERMSIG(status));
		}
		while(usr2){
			pause();
		}
		printf("Received %d SIGUSR1 signals\n", counter);
		printf("Signals send: %d\n", sends);
	}
	return 0;
}