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

void* io_thread(void* io){
    (void)io;
    int out; 
    handle((out = dup(pipeToServer[1])), == -1, "Can't set pipe to server", 1);
    int in;
    handle((in = dup(pipeFromServer[0])), == -1, "Cant' set pipe from server", 1);
    struct pollfd mz_poll[2];
    mz_poll[0] = (struct pollfd){ 
        .fd = 0, 
        .events = POLLIN | POLLHUP, 
        .revents = 0};
    mz_poll[1] = (struct pollfd){
        .fd = in, 
        .events = POLLIN | POLLHUP, 
        .revents = 0};
    while(not_finished){
        msg_t message;
        int ready = poll(mz_poll, 2, 3000);
        if(ready > 0){
            if(mz_poll[0].revents & POLLIN){
                read(0, message.m, sizeof(message.m));
                handle(write(out, message.m, sizeof(message.m)), == -1, "Can't send message to main thread", 0);
                //fprintf(stderr, "SEND TO MAIN THREAD");
            }else if(mz_poll[1].revents & POLLIN){
                int readed;
                handle((readed = read(in, &message, sizeof(msg_t))), == -1, "Can't read message", 0);
                if(readed > 0){
                fprintf(stdout, "<%s>: %s", message.id, message.m);
                }
            }
            if(mz_poll[0].revents & POLLHUP){
                printf("End of client communication. Closed by user.\n");
                pthread_exit(0);
            }
        }
        handle(ready == -1, && (errno != EINTR) , "Error in poll", 0);
    }
    pthread_exit(0);
}







void exitfun(void){
    not_finished = 0;
    if(socket_fd == 0) return;
    handle(shutdown(socket_fd, SHUT_RDWR), == -1, "Can't shudown socket.", 1);
    handle(close(socket_fd), == -1, "Can't close socket fd", 1);
    socket_fd = 0;
    handle(errno, >0, "Error: ",1);
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
    handle(pipe(pipeToServer), == -1, "Can't create pipe", 0);
    handle(pipe(pipeFromServer), == -1, "Can't create pipe", 0);
    pthread_t child;
    handle((errno = pthread_create(&child, NULL, &io_thread, NULL)), >0, "Can't create thread.", 1);
    struct sockaddr_un unix_addr;
    struct sockaddr_in net_addr;
    int from;
    handle((from = dup(pipeFromServer[1])), == -1, "Can't duplicate", 0);
    int to;
    handle((to = dup(pipeToServer[0])), == -1, "Can't duplicate", 0);
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
            handle(argc, != 4, "Bad configuration for l argument", 1);
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
            handle(argc, != 5, "Bad configuration for r argument", 1);
            break;
    }
    //printf("FD: %d\n", socket_fd);
    handle(connect(socket_fd, addr, size_addr), == -1, "Can't connect to server.", 1);
    //signal(SIGPIPE, &signaled);
    signal(SIGINT, &signaled);
    atexit(&exitfun);

    struct pollfd mz_poll[2];
    mz_poll[0] = (struct pollfd){
        .fd = socket_fd, 
        .events = POLLIN, 
        .revents = 0};
    mz_poll[1] = (struct pollfd){
        .fd = to, 
        .events = POLLIN, 
        .revents = 0};
    while(not_finished){
        int ready = poll(mz_poll, 2, 30000);
        handle(ready == -1, && errno != EINTR , "Error on server communication poll.", 0);
        if(ready > 0){
            if(mz_poll[0].revents & POLLIN){
                int readed = -1;
                handle((readed = read(socket_fd, &message, sizeof(msg_t))), == -1, "Can't receive message", 1);
                handle(write(from, &message, sizeof(msg_t)), == -1, "Can't send message to thread.", 1);
            }
           if(mz_poll[1].revents & POLLIN){
                msg_t message;
                memset(&message, 0, sizeof(msg_t));
                read(to, message.m, sizeof(message.m));
                strncpy(message.id, name, sizeof(name));
                //fprintf(stderr, "SEND TO SERVER: %s, %s", message.id, message.m);
                handle(write(socket_fd, &message, sizeof(msg_t)), == -1, "Can't send message to server from main thread.", 1);
           }
        }
    }
    void* status;
    handle((errno = pthread_join(child, &status)), >0, "Can't join child",1);
    child = 0;
    exitfun();
    pthread_exit(0);
}