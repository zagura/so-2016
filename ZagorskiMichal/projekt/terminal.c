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


#define BUF_SIZE 4096


char name[BUF_SIZE];
int port = -1;

struct connection{
    char arguments[1024];
    int interval_s;
    char command[10];
}__attribute__((packed));

typedef struct connection connection_t;

int pipeToServer[2];
int pipeFromServer[2];
int not_finished = 1;
int socket_fd;

struct sockaddr* addr;
unsigned int size_addr;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int time_interval = 0;

void* io_thread(void* io){
    (void)io;
    int out; 
    handle((out = dup(pipeToServer[1])), == -1, "Can't set pipe to server", 1);
    int in;
    handle((in = dup(pipeFromServer[0])), == -1, "Cant' set pipe from server", 1);
    struct pollfd mz_poll[2];
    mz_poll[0] = (struct pollfd){ 
        .fd = 0, 
        .events = POLLIN | POLLRDHUP, 
        .revents = 0};
    mz_poll[1] = (struct pollfd){
        .fd = in, 
        .events = POLLIN | POLLRDHUP, 
        .revents = 0};
    time_t last_time = 0;
    while(not_finished){
        int ready = poll(mz_poll, 2, 30000);
        if(ready > 0){
/*            if(mz_poll[0].revents & POLLIN){
                read(STDIN_FILENO, message.m, sizeof(message.m));
                handle(write(out, message.m, sizeof(message.m)), == -1, "Can't send message to main thread", 0);
                //fprintf(stderr, "SEND TO MAIN THREAD");
            }*/if(mz_poll[1].revents & POLLIN){
                int readed;
                char message[4096];
                handle((readed = read(in, &message, sizeof(message))), == -1, "Can't read message", 0);
                if(readed > 0){
                    time_t curr_time = time(NULL);
                    if(curr_time - last_time > time_interval){
                        char* timed = ctime((time_t*)&curr_time);
                        if(timed[strlen(timed) -1] == '\n'){
                            timed[strlen(timed) -1] = '\0';
                        }
                        fprintf(stdout, "\n\nTime: %s : \n", timed);
                    }
                    fprintf(stdout, "%s\n", message);
                    last_time = time(NULL);
                }
            }
            if(mz_poll[0].revents & POLLRDHUP || mz_poll[1].revents & POLLRDHUP){
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
    handle(shutdown(socket_fd, SHUT_RDWR), == -1, "Can't shudown socket.", 1);
    handle(close(socket_fd), == -1, "Can't close socket fd", 1);
    socket_fd = 0;
}

void signaled(int signo){
    (void)signo;
    exitfun();
}

struct sockaddr_in net_addr;



int main(int argc, char** argv){
    /*  ARGS
        1. IP Address
        2. Port
        3. service
        4. time interval (s)
        5. additional arfuments
    */
    fprintf(stdout, "Table of available services:\n");
    fprintf(stdout, "1. (a) w - 'w' command\n");
    fprintf(stdout, "2. (b) inotifywait - inotify watch command; additional args needed (path to observed directory)\n");
    fprintf(stdout, "3. (c) ps - ps command on remote host\n");
    fprintf(stdout, "4. (d) df - df command on remote host's filesystem (caled only once, time interval skipped)\n");
    fprintf(stdout, "5. (e) ifconfig - ifconfig command on remote host (caled only once, time interval skipped)\n");
    fprintf(stdout, "6. (f) vmstat - memory statistics command on remote host\n");
    handle(argc, != 6, "wrong amount of arguments: <IP Adress> <Port> <Service> <time interval in seconds> <additional arguments>|<null>",0)
    handle(argc , >6, "Too many arguments.", 1);
    handle(argc , <6, "Too few arguments.", 1);
    strncpy(name, argv[1], strlen(argv[1]));
    //type = (argv[2] == 'l' ? 0 : (argv[2] == 'r' ? 1 : handle(0, == 0, "Wrong signature for type of socket \'l\'(local) or \'r\'(remote).", 1)));


    handle(pipe(pipeToServer), == -1, "Can't create pipe", 0);
    handle(pipe(pipeFromServer), == -1, "Can't create pipe", 0);
    pthread_t child;
    handle((errno = pthread_create(&child, NULL, &io_thread, NULL)), >0, "Can't create thread.", 1);

    socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    memset(&net_addr, 0, sizeof(net_addr));
    net_addr.sin_family = AF_INET;
    net_addr.sin_port = htons(atoi(argv[2]));
    net_addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr = (struct sockaddr*)&net_addr;
    size_addr = sizeof(net_addr);

    signal(SIGINT, &signaled);

    struct pollfd mz_poll[2];

    connection_t communicate;
    handle(connect(socket_fd, (struct sockaddr*)&(net_addr), size_addr), == -1, "can't connect to remote host.", 1);

    atexit(&exitfun);
    strncpy(communicate.command, argv[3], sizeof(communicate.command));
    communicate.interval_s =atoi(argv[4]);
    strncpy(communicate.arguments, argv[5], sizeof(communicate.arguments));
    handle(write(socket_fd, &communicate, sizeof(communicate)), == -1, "Can't send parameters of service.", 1);
    
    mz_poll[0] = (struct pollfd){
        .fd = socket_fd, 
        .events = POLLIN | POLLRDHUP,
        .revents = 0};
    int from;
    handle((from = dup(pipeFromServer[1])), == -1, "Can't duplicate", 0);
    int to;
    handle((to = dup(pipeToServer[0])), == -1, "Can't duplicate", 0);
    mz_poll[1] = (struct pollfd){
        .fd = to, 
        .events = POLLIN | POLLRDHUP, 
        .revents = 0};
    time_interval = atoi(argv[4]);
    while(not_finished){
        int ready = poll(mz_poll, 2, time_interval*500);
        handle(ready == -1 &&, (errno != EINTR), "Error on terminal communication poll.", 0);
        if(ready > 0){
            char message[4096];
            if(mz_poll[0].revents & POLLRDHUP || mz_poll[1].revents & POLLRDHUP ){
                not_finished = 0;
            }
            int readed = -2;
            if(mz_poll[0].revents & POLLIN){
                memset(&message, 0, sizeof(message));
                handle((readed = read(socket_fd, &message, sizeof(message))), == -1, "Can't receive message from server", 0);
                write(from, &message, sizeof(message));
                if(readed == 0) not_finished = 0;
            }
/*           if(mz_poll[1].revents & POLLIN){
                memset(&message, 0, sizeof(message));
               // read(to, message.m, sizeof(message.m));

               // strncpy(message.id, name, sizeof(name));
                //fprintf(stderr, "SEND TO SERVER: %s, %s", message.id, message.m);
                //handle(sendto(socket_fd, &message, sizeof(msg_t), 0, addr, size_addr), == -1, "Can't send message to server from main thread.", 1);
           }*/
        }
    }
    void* status;
    handle((errno = pthread_join(child, &status)), >0, "Can't join child",1);
    child = 0;
    exitfun();
    pthread_exit(0);
}
