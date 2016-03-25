#define _POSIX_C_SOURCE 199309L

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

static int counter = 0, repetitions = 1, modifier = -1, max_num = -1;
static char* str = NULL;
static char* str_r = NULL;
static char reversed[100];

void println(const char* printed, int n){
	int i = 0;
	printf("\n");
	for(i = 0; i < n; i++){
		printf("%s,", printed);
	}
	printf("\n");
}

char* reverse(const char* word){
	char* rev = strcpy(reversed, word);
	char* end = (rev + strlen(rev)) - 1;
	char* begin = rev;
	char tmp;
	while(end > begin){
		tmp = *begin;
		*begin = *end;
		*end = tmp;
		end--;
		begin++;
	}
//	fprintf(stderr, "\n-----\n%s\n%s\n----\n", rev, reversed);
	return rev;
}


void int_handler(int signo){
	if(signo == SIGINT){
		fprintf(stdout, "\nOdebrano sygnał SIGINT\n");
		fprintf(stderr, "Counter value: %d\n", counter);
		exit(EXIT_SUCCESS);
	}
	fprintf(stderr, "Catched %d signal\n", signo);
}


void handler(int signo){
	switch(counter % 3){
		case 0:
			println(str, repetitions);
			if(repetitions == max_num || repetitions == 1){
				modifier *= -1;
			}
			break;
		case 1:
			println(str_r, repetitions);
			break;
		case 2:
			println(str, repetitions);
			repetitions += modifier;
			break;
		default:
			fprintf(stderr, "Wrong counter arg\n");
			break;
	}
	counter++;
}


int main(int argc, char** argv){
	char param[] = "max-num";
	int arg = 0;
	if(argc > 3){
		fprintf(stderr, "Invalid number of given arguments!\n");
		return -1;
	}
	for(arg = 1; arg < argc; arg++){
		if(strncmp(param, argv[arg], strlen(param)) == 0 || 
			strncmp(param, (argv[arg]) + 1, strlen(param)) == 0 ||
			strncmp(param, (argv[arg]) + 2, strlen(param)) == 0){
			int index = strlen(param);
			if(argv[arg][0] == '-'){
				index++;
			}
			if(argv[arg][1] == '-'){
				index++;
			}
			if(argv[arg][index] == '='){
				index++;
			}
			max_num = atoi(argv[arg] + index);
		}
		else{
			str = argv[arg];
		}
	}
	if(str == NULL || max_num < 0){
		fprintf(stderr, "Invalid arguments\n");
		return EXIT_FAILURE;
	}
	str_r = reverse(str);
	struct sigaction stop;
	stop.sa_handler = &handler;
	stop.sa_flags = 0;
	if(sigfillset(&(stop.sa_mask)) == - 1){
		perror("Mask set");
	}
	sigdelset(&(stop.sa_mask), SIGTSTP);
	if(sigaction(SIGTSTP, &stop, NULL) == -1){
		perror("\nSigaction");
	}
	signal(SIGINT, &int_handler);
	while(1){
		pause();
	}
	return 0;
}

