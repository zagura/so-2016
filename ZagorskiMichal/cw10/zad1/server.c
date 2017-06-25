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

#define CLIENTS 20

char path[BUF_SIZE];
int port = 0;
struct msg{
    char id[BUF_SIZE];
    char m[BUF_SIZE];
}__attribute__((packed));
typedef struct msg msg_t;
struct sockaddr_un unix_addr;
struct sockaddr_in net_addr;

int local;
int global;
int not_finished = 1;


union mz_sockaddr_com{
    struct sockaddr_un un;
    struct sockaddr_in ip;
};
typedef union mz_sockaddr_com mz_sock;
socklen_t unix_len = sizeof(unix_addr);
socklen_t net_len = sizeof(net_addr);


struct client{
    char id[BUF_SIZE];
    time_t last_access;
    int registred;
    int type;
    mz_sock addr;
    socklen_t len;
};

typedef struct client client_t;


client_t clients[CLIENTS];
void exitfun(void){
    not_finished = 0;
    if(local != 0){
        handle(shutdown(local, SHUT_RDWR), == -1, "Can't shudown socket.", 1);
        handle(close(local), == -1, "Can't close socket fd", 1);
        handle(unlink(path), == -1, "Can't unlink socket", 1);
        local = 0;
    }
    if(global != 0){
       // handle(shutdown(global, SHUT_RDWR), == -1, "Can't shudown socket.", 1);
        handle(close(global), == -1, "Can't close socket fd", 1);
        global = 0;
    }
}

void signaled(int signo){
    (void)signo;
    exitfun();
}

void clean_register(int size, time_t current_time);
void received(msg_t* message, int size, time_t current_time, int type, mz_sock src, socklen_t len);


int main(int argc, char** argv){
    handle(argc, != 3, "Wrong amount of arguments: program <port> <socket_path>", 1);
    port = atoi(argv[1]);

    strncpy(path, argv[2], UNIX_PATH_MAX);
    memset(&net_addr, 0, sizeof(net_addr));
    memset(&unix_addr, 0, sizeof(unix_addr));
    net_addr.sin_family = AF_INET;
    net_addr.sin_port = htons(port);
    net_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    unix_addr.sun_family = AF_UNIX;
    strncpy(unix_addr.sun_path, argv[2], UNIX_PATH_MAX -1);
    unix_addr.sun_path[UNIX_PATH_MAX-1] = '\0';
    handle((local = socket(AF_UNIX, SOCK_DGRAM,0)), == -1, "Can't create socket.",1);
    
    global = socket(AF_INET, SOCK_DGRAM, 0);
    fprintf(stderr, "local: %d\n", local);
    handle(bind(global, (struct sockaddr*)&net_addr, sizeof(net_addr)), == -1, "Can't bind local address.", 1);
    handle(bind(local, (struct sockaddr*)&unix_addr, sizeof(unix_addr)), == -1, "Can't bind local unix address.", 1);
    signal(SIGINT, &signaled);
    atexit(&exitfun);

    
    clean_register(CLIENTS, time(NULL));
    int counter = 0;
    struct pollfd mz_poll[2];
    mz_poll[0] = (struct pollfd){ 
        .fd = local , 
        .events = POLLIN, 
        .revents = 0};
    mz_poll[1] = (struct pollfd){
        .fd = global, 
        .events = POLLIN, 
        .revents = 0};
    msg_t message;
    while(not_finished){
        int ready = poll(mz_poll, 2, 30000);
        handle(ready, ==-1 && (errno != EINTR), "Error on io poll.", 0);
        int type = -1;
        mz_sock src;
        socklen_t len;
        memset(&src, 0, sizeof(src));
        if(ready > 0){
            if(mz_poll[0].revents & POLLIN){
                //UNIX socket
                len = unix_len;
                memset(&message, 0, sizeof(msg_t));
                handle(recvfrom(mz_poll[0].fd, &message, sizeof(msg_t), 0, (struct sockaddr*)&(src.un) , &len), == -1, "Can't receive message unix", 1);
            //    len = sizeof(src.un);
		   //   fprintf(stderr, "UNIX: %s len: %d path: %s family: %d", message.m, len, src.un.sun_path, src.un.sun_family);
                type = 0;
                counter++;
            }
            if(mz_poll[1].revents & POLLIN){
           //     fprintf(stderr, "Received msg from remote client\n");
                len = net_len;
                memset(&message, 0, sizeof(msg_t));
                handle(recvfrom(mz_poll[1].fd,  &message, sizeof(msg_t), 0, (struct sockaddr*)&src , &len), == -1, "Can't receive message ip", 1);
                type = 1;
                counter++;
	//	fprintf(stderr, "REMOTE: %s, len: %d", message.m, len);
            }
            received(&message, CLIENTS, time(NULL), type, src, len);
        }if(ready == 0 || counter > 5){
            counter = 0;
            clean_register(CLIENTS, time(NULL));
        }
        handle(memset(&message, 0, sizeof(message)), == NULL, "MEMSET: ",0);
    }

    exitfun();
	return 0;
}


void clean_register(int size, time_t current_time){
    for(int i = 0; i < size; i++){
        if(clients[i].registred){
            time_t last_access = clients[i].last_access;
            if((current_time - last_access) >= 10){
                fprintf(stderr, "Removing client %s from index: %d\n", clients[i].id, i);
                memset(&clients[i], 0, sizeof(client_t));
            }
        }
    }
}

void received(msg_t* message, int size, time_t current_time, int type, mz_sock src, socklen_t len){
    //fprintf(stderr, "\n\n RECEIVED CALL:\n %s  :: %s ;; \n", message->id, message->m);
    char* el = (message->m);
    for(int i = 0; i < BUF_SIZE; i++){
        if((*el) == '\n' || (*el) == '\0'){
            memset(++el, 0, BUF_SIZE - i - 2);
            break;
        }
        el++;
    }
    fprintf(stderr, "%s:%s", message->id, message->m);
    int res = 0;
    for(int i = 0; i < size; i++){
        if(clients[i].registred){
            char* c_name = clients[i].id;
            if(strncmp(c_name, message->id, BUF_SIZE) == 0){
                clients[i].last_access = current_time;
                fprintf(stderr, "Client at %i updated time\n", i);
                res = 1;
            }else{
                int d_type = clients[i].type;
                if(d_type == 0){
                        //UNIX DESTINATION
                        //clients[i].addr.un.sun_family = AF_UNIX;
                        //fprintf(stderr, "SEND TO UNIX: family: %d path: %s len: %d, id: %s\n", clients[i].addr.un.sun_family, clients[i].addr.un.sun_path, clients[i].len, clients[i].id);
                        handle(sendto(local, message, sizeof(msg_t),0, (struct sockaddr*)&(clients[i].addr.un), clients[i].len), == -1 && errno != ENOENT, "Can't send message", 0);
                }else{
                    //fprintf(stderr, "SEND TO IP: family: %d Port: %d len: %d, id: %s\n", clients[i].addr.ip.sin_family, clients[i].addr.ip.sin_port ,clients[i].len, clients[i].id);
                    handle(sendto(global, message, sizeof(msg_t),0, (struct sockaddr*)&(clients[i].addr.ip), clients[i].len), == -1, "Can't send message", 0);
                }
            }
        }
    }
    if(res == 0){
        for(int i = 0; i < size; i++){
            if(clients[i].registred == 0){
                strncpy(clients[i].id, message->id, BUF_SIZE);
                clients[i].last_access = current_time;
                clients[i].type = type;
                clients[i].addr = src;
                clients[i].registred = 1;
                clients[i].len = len;
                fprintf(stderr, "Registration of client %s at %d\n", message->id, i);
                int d_type = clients[i].type;
                if(d_type == 0){
                        //UNIX DESTINATION
                        //clients[i].addr.un.sun_family = AF_UNIX;
                       // fprintf(stderr, "SEND TO UNIX: family: %d path: %s len: %d, id: %s\n", clients[i].addr.un.sun_family, clients[i].addr.un.sun_path, clients[i].len, clients[i].id);
                        handle(sendto(local, message, sizeof(msg_t),0, (struct sockaddr*)&(clients[i].addr.un), clients[i].len), == -1, "Can't send message", 0);
                }else{
                   // fprintf(stderr, "SEND TO IP: family: %d Port: %d len: %d, id: %s\n", clients[i].addr.ip.sin_family, clients[i].addr.ip.sin_port ,clients[i].len, clients[i].id);
                    handle(sendto(global, message, sizeof(msg_t),0, (struct sockaddr*)&(clients[i].addr.ip), clients[i].len), == -1, "Can't send message", 0);
                }
                break;
            }
        }
    }
}