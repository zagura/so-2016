#define _XOPEN_SOURCE 700
#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE

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

#define UNIX_PATH_MAX 107
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

struct sockaddr* addr;
unsigned int size_addr;
msg_t message;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
char* line = NULL;
char path[UNIX_PATH_MAX];
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
            }if(mz_poll[1].revents & POLLIN){
                int readed;
                handle((readed = read(in, &message, sizeof(msg_t))), == -1, "Can't read message", 0);
                if(readed > 0){
                fprintf(stdout, "<%s>: %s", message.id, message.m);
                }
            }
            if(mz_poll[0].revents & POLLHUP || mz_poll[1].revents & POLLHUP){
                printf("End of client communication. Closed by user.\n");
                not_finished = 0;
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
    if(type == 'l')
    handle(shutdown(socket_fd, SHUT_RDWR), == -1, "Can't shudown socket.", 1);
    handle(close(socket_fd), == -1, "Can't close socket fd", 1);
    if(type == 'l'){
        handle(unlink(path), == -1, "can't unlink unix socket", 1);
    }
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
            socket_fd = socket(AF_LOCAL, SOCK_DGRAM, 0);
            memset(&unix_addr, 0, sizeof(unix_addr));
            unix_addr.sun_family = AF_LOCAL;
            strncpy(unix_addr.sun_path, argv[3], sizeof(unix_addr.sun_path) -1);
            unix_addr.sun_path[UNIX_PATH_MAX-1] = '\0';
            addr = (struct sockaddr*)&unix_addr;
            size_addr = sizeof(unix_addr);
            struct sockaddr_un local_addr; 
            socklen_t local_len;
            memset(&local_len, 0, sizeof(local_len));
            memset(&local_addr, 0, sizeof(local_addr));
            local_addr.sun_family = AF_UNIX;
            sprintf(path, "%s-%d", name, getpid());
            strncpy(local_addr.sun_path, path, UNIX_PATH_MAX-1);
            local_addr.sun_path[UNIX_PATH_MAX-1] = '\0';
            local_len = sizeof(local_addr);

            //handle(getsockname(socket_fd, (struct sockaddr*)&local_addr, &local_len), == -1, "Can't get local address", 0);
            unlink(local_addr.sun_path);
            handle(bind(socket_fd, (struct sockaddr*)&local_addr, local_len), == -1, "Can't bind socket", 0);
            break;
        case 'r':
            socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
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

    mz_poll[0] = (struct pollfd){
        .fd = socket_fd, 
        .events = POLLIN, 
        .revents = 0};
    int from;
    handle((from = dup(pipeFromServer[1])), == -1, "Can't duplicate", 0);
    int to;
    handle((to = dup(pipeToServer[0])), == -1, "Can't duplicate", 0);
    mz_poll[1] = (struct pollfd){
        .fd = to, 
        .events = POLLIN, 
        .revents = 0};
    while(not_finished){
        msg_t message;
        int ready = poll(mz_poll, 2, 30000);
        handle(ready == -1 &&, (errno != EINTR), "Error on server communication poll.", 0);
        if(ready > 0){
            if(mz_poll[0].revents & POLLIN){
                memset(&message, 0, sizeof(msg_t));
                handle(recvfrom(socket_fd, &message, sizeof(msg_t), 0, NULL, 0), == -1, "Can't receive message", 1);
                write(from, &message, sizeof(msg_t));
            }
           if(mz_poll[1].revents & POLLIN){
                memset(&message, 0, sizeof(msg_t));
                read(to, message.m, sizeof(message.m));
                strncpy(message.id, name, sizeof(name));
                //fprintf(stderr, "SEND TO SERVER: %s, %s", message.id, message.m);
                handle(sendto(socket_fd, &message, sizeof(msg_t), 0, addr, size_addr), == -1, "Can't send message to server from main thread.", 1);
           }
           if(mz_poll[0].revents & POLLHUP || mz_poll[1].revents & POLLHUP ){
            not_finished = 0;
           }
        }
    }

    void* status;
    handle((errno = pthread_join(child, &status)), >0, "Can't join child",1);
    child = 0;
    exitfun();
    pthread_exit(0);
}