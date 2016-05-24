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

#define LISTEN 50

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

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
char* line = NULL;
msg_t message;

void send_message(){  
    msg_t message;
    strncpy(message.id, name, sizeof(name));
    strncpy(message.m, buffer1, sizeof(buffer1));
    handle(write(socket_fd, &message, sizeof(msg_t)), == -1, "Can't send message to server", 1);
}



void* writer(void* args){
    (void)args;
    memset(&buffer2, 0, BUF_SIZE);
    while(not_finished){
        pthread_mutex_lock(&mutex);
        pthread_cond_wait(&cond, &mutex);
        fprintf(stderr, "<%s> : %s", message.id, message.m);
        memset(&message, 0, sizeof(msg_t));
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(0);
}

void* reader(void* args){
    size_t size = 0;
    (void)args;
    while(getline(&line, &size, stdin) != -1 && not_finished){
        strncpy(buffer1, line, BUF_SIZE);
        free(line);
        line = NULL;
        size = 0;
        send_message();
    }
    if(line != NULL){
        free(line);
        line = NULL;
    }
    pthread_exit(0);
}

void exitfun(void){
    not_finished = 0;
    if(socket_fd == 0) return;
    handle(shutdown(socket_fd, SHUT_RDWR), == -1, "Can't shudown socket.", 1);
    handle(close(socket_fd), == -1, "Can't close socket fd", 1);
    socket_fd = 0;
    pthread_mutex_lock(&mutex);
    pthread_mutex_lock(&mutex);
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutex);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    handle(errno, >0, "Error: ",1);
    exit(0);
}

void signaled(int signo){
    (void)signo;
    exitfun();
}






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

    pthread_t r_child, w_child;
    handle((errno = pthread_create(&r_child, NULL, &reader, NULL)), >0, "Can't create thread.", 1);
    handle((errno = pthread_create(&w_child, NULL, &writer, NULL)), >0, "Can't create thread.", 1);
    struct sockaddr_un unix_addr;
    struct sockaddr_in net_addr;

    // SET SOCKET
    switch(type){
        case 'l':
            socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
            memset(&unix_addr, 0, sizeof(unix_addr));
            unix_addr.sun_family = AF_UNIX;
            strncpy(unix_addr.sun_path, argv[3], sizeof(unix_addr.sun_path) -1);
            unix_addr.sun_path[UNIX_PATH_MAX-1] = '\0';
            addr = (struct sockaddr*)&unix_addr;
            size_addr = sizeof(unix_addr);
            break;
        case 'r':
            socket_fd = socket(AF_INET, SOCK_STREAM, 0);
            memset(&net_addr, 0, sizeof(net_addr));
            net_addr.sin_family = AF_INET;
            net_addr.sin_port = htons(atoi(argv[4]));
            int res = inet_pton(AF_INET, argv[3], &net_addr.sin_addr);
            handle(res, <= 0, "Can\'t validate address", 1);
            addr = (struct sockaddr*)&net_addr;
            size_addr = sizeof(net_addr);
        break;
    }printf("FD: %d\n", socket_fd);
    handle(connect(socket_fd, addr, size_addr), == -1, "Can't connect to server.", 1);
    //signal(SIGPIPE, &signaled);
    signal(SIGINT, &signaled);
    //atexit(&exitfun);
    struct pollfd mz_poll = (struct pollfd){
        .fd = socket_fd, 
        .events = POLLIN, 
        .revents = 0};
    pthread_mutex_lock(&mutex);
    while(not_finished){
        int ready = poll(&mz_poll, 1, 30000);
        handle(ready, ==-1, "Error on server communication poll.", 0);
        if(ready > 0){
            if(mz_poll.revents & POLLIN){
                int readed = -1;
                handle((readed = read(socket_fd, &message, sizeof(msg_t))), == -1, "Can't receive message", 1);
                if(readed > 0){
                    pthread_cond_broadcast(&cond);
                    pthread_mutex_unlock(&mutex);
                    pthread_mutex_lock(&mutex);
                    pthread_cond_wait(&cond, &mutex);
                }
            }
        }
    }
    pthread_mutex_unlock(&mutex);
    void* status;
    handle((errno = pthread_join(r_child, &status)), >0, "Can't join child",1);
    r_child = 0;
    handle((errno = pthread_join(w_child, &status)), >0, "Can't join child",1);
    w_child = 0;
    exitfun();
	pthread_exit(0);
}