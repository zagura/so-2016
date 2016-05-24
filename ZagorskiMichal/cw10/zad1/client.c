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
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/ip.h>
#include <poll.h>
#include <arpa/inet.h>


#define handle(fun, val, text, exited)                                   \
                        if(fun val){                                     \
                            fprintf(stderr, "Line %d: ", __LINE__);      \
                            perror(text "\n");                           \
                            if(exited){                                  \
                                exit(EXIT_FAILURE);                      \
                            }                                            \
                            errno = 0;                                   \
                        }
#define UNIX_PATH_MAX 108
#define BUF_SIZE 256


char name[BUF_SIZE];
char type = 'n';
int port = -1;

struct msg{
    char id[BUF_SIZE];
    char m[BUF_SIZE];
}__attribute__((packed));
typedef struct msg msg_t;
int pipeToServer[2];
int pipeFromServer[2];
int not_finished = 1;
int socket_fd;
char buffer1[BUF_SIZE];
char buffer2[BUF_SIZE];
struct sockaddr* addr;
unsigned int size_addr;
msg_t message;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
char* line = NULL;

void* io_thread(void* io){
	(void)io;
	int out = dup(pipeToServer[1]);
	int in = dup(pipeFromServer[0]);
	//close(pipeFromServer[1]);
	//close(pipeToServer[0]);
	struct pollfd mz_poll[2];
    mz_poll[0] = (struct pollfd){ 
        .fd = 0, 
        .events = POLLIN, 
        .revents = 0};
    mz_poll[1] = (struct pollfd){
        .fd = in, 
        .events = POLLIN, 
        .revents = 0};
	while(not_finished){
		msg_t message;
		int ready = poll(mz_poll, 2, 3000);
		if(ready > 0){
			if(mz_poll[0].revents & POLLIN){
				read(0, message.m, sizeof(message.m));
				write(out, message.m, sizeof(message.m));
				//fprintf(stderr, "SEND TO MAIN THREAD");
			}else if(mz_poll[1].revents & POLLIN){
				int readed = read(in, &message, sizeof(msg_t));\
				if(readed > 0){
				fprintf(stdout, "<%s>: %s", message.id, message.m);
				}
			}
		}
	}
	
	pthread_exit(0);
}






void exitfun(void){
    not_finished = 0;
    if(socket_fd == 0) return;
    //handle(shutdown(socket_fd, SHUT_RDWR), == -1, "Can't shudown socket.", 1);
    handle(close(socket_fd), == -1, "Can't close socket fd", 1);
  // handle(unlink(path), == -1, "can't unlink unix socket", 1);
    socket_fd = 0;
}

void signaled(int signo){
    (void)signo;
    exitfun();
}
struct sockaddr_un unix_addr;
struct sockaddr_in net_addr;





int main(int argc, char** argv){
    /*  ARGS
        1. CLIENT's ID
        2. SERVER's TYPE l or r
        3. PATH to FILE or ip address
        4. (only r) port
    */
    handle(argc , >5, "Too many arguments.", 1);
    handle(argc , <4, "Too few arguments.", 1);
    strncpy(name, argv[1], strlen(argv[1]));
    //type = (argv[2] == 'l' ? 0 : (argv[2] == 'r' ? 1 : handle(0, == 0, "Wrong signature for type of socket \'l\'(local) or \'r\'(remote).", 1)));
    type = argv[2][0];
    handle(pipe(pipeToServer), == -1, "Can't create pipe", 0);
    handle(pipe(pipeFromServer), == -1, "Can't create pipe", 0);
    pthread_t child;
    handle((errno = pthread_create(&child, NULL, &io_thread, NULL)), >0, "Can't create thread.", 1);


    // SET SOCKET
    switch(type){
        case 'l':
            socket_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
            memset(&unix_addr, 0, sizeof(unix_addr));
            unix_addr.sun_family = AF_UNIX;
            strncpy(unix_addr.sun_path, argv[3], sizeof(unix_addr.sun_path) -1);
            unix_addr.sun_path[UNIX_PATH_MAX-1] = '\0';
            addr = (struct sockaddr*)&unix_addr;
            size_addr = sizeof(unix_addr);
            break;
        case 'r':
            socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
            memset(&net_addr, 0, sizeof(net_addr));
            net_addr.sin_family = AF_INET;
            net_addr.sin_port = htons(atoi(argv[4]));
            net_addr.sin_addr.s_addr = inet_addr(argv[3]);
            addr = (struct sockaddr*)&net_addr;
            size_addr = sizeof(net_addr);
        break;
    }
    signal(SIGINT, &signaled);
    atexit(&exitfun);
    struct pollfd mz_poll[2];
    while(not_finished){
    mz_poll[0] = (struct pollfd){
        .fd = socket_fd, 
        .events = POLLIN, 
        .revents = 0};
    int from = dup(pipeFromServer[1]);
    int to = dup(pipeToServer[0]);
    //close(pipeFromServer[0]);
    //close(pipeToServer[1]);
    mz_poll[1] = (struct pollfd){
        .fd = to, 
        .events = POLLIN, 
        .revents = 0};
        int ready = poll(mz_poll, 2, 30000);
        handle(ready, ==-1, "Error on server communication poll.", 0);
        if(ready > 0){
            if(mz_poll[0].revents & POLLIN){
                handle(recvfrom(socket_fd, &message, sizeof(msg_t), 0, NULL, 0), == -1, "Can't receive message", 1);
                write(from, &message, sizeof(msg_t));
            }
           if(mz_poll[1].revents & POLLIN){
		msg_t message;
		read(to, message.m, sizeof(msg_t));
		strncpy(message.id, name, sizeof(name));
		//fprintf(stderr, "SEND TO SERVER: %s, %s", message.id, message.m);
		handle(sendto(socket_fd, &message, sizeof(msg_t), 0, addr, size_addr), == -1, "Can't receive message", 1);
           }
        }
    }
    void* status;
    handle((errno = pthread_join(child, &status)), >0, "Can't join child",1);
    child = 0;
    exitfun();
    return 0;
}