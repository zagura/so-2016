#define _GNU_SOURCE
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
#define LISTEN 50

char path[BUF_SIZE];
int port = 0;
struct msg{
    char id[BUF_SIZE];
    char m[BUF_SIZE];
}__attribute__((packed));
typedef struct msg msg_t;
struct sockaddr_un unix_addr;
struct sockaddr_in net_addr;
socklen_t unix_len = sizeof(unix_addr);
socklen_t net_len = sizeof(net_addr);
int local;
int global;
int not_finished = 1;

struct client{
    time_t last_access;
    int registred;
    int type;
    int fd;
    int confirmed;
};
typedef struct client client_t;


client_t clients[CLIENTS];
void exitfun(void){
    not_finished = 0;
    for(int i = 0; i < CLIENTS; i++){
        if(clients[i].registred){
        handle(shutdown(clients[i].fd, SHUT_RDWR), == -1, "Can't shutdown client's socket.", 0);
        handle(close(clients[i].fd), == -1, "Can't close socket.", 0);
    }
    }
                
    if(local != 0){
        handle(shutdown(local, SHUT_RDWR), == -1, "Can't shudown socket.", 1);
        handle(close(local), == -1, "Can't close socket fd", 0);
        //fprintf(stderr, "PATH:  %s  END",path);
        handle(unlink(path), == -1, "Can't unlink socket", 0);
        local = 0;
    }
    if(global != 0){
        handle(shutdown(global, SHUT_RDWR), == -1, "Can't shudown socket.", 0);
        handle(close(global), == -1, "Can't close socket fd", 0);
        global = 0;
    }
}

void signaled(int signo){
    (void)signo;
    exitfun();
}

int clean_register(time_t current_time, int size);
void received(time_t current_time, int type, int src);
int register_sock(time_t current_time, int type, int src);


int main(int argc, char** argv){
    handle(argc, != 3, "Wrong amount of arguments: program <port> <socket_path>", 1);
    port = atoi(argv[1]);
    strncpy(path, argv[2], BUF_SIZE);

    memset(&net_addr, 0, sizeof(net_addr));
    memset(&unix_addr, 0, sizeof(unix_addr));
    net_addr.sin_family = AF_INET;
    net_addr.sin_port = htons(port);
    net_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    unix_addr.sun_family = AF_UNIX;
    strncpy(unix_addr.sun_path, argv[2], UNIX_PATH_MAX -1);
    unix_addr.sun_path[UNIX_PATH_MAX-1] = '\0';
    handle((local = socket(AF_UNIX, SOCK_STREAM,0)), == -1, "Can't create socket.",1);
    
    handle((global = socket(AF_INET, SOCK_STREAM,IPPROTO_TCP)), == -1, "Can't create socket.",1);
    handle(setsockopt(global, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)), == -1, "Can't set reusable address.", 0);
    //fprintf(stderr, "local: %d\n", local);
    handle(bind(global, (struct sockaddr*)&net_addr, net_len), == -1, "Can't bind global address.", 1);
    handle(bind(local, (struct sockaddr*)&unix_addr, unix_len), == -1, "Can't bind local unix address.", 1);
    signal(SIGINT, &signaled);
    atexit(&exitfun);

    handle(listen(local, LISTEN), == -1, "Error in listen operation", 1);
    handle(listen(global, LISTEN), == -1, "Error in listen operation", 1);

    clean_register(CLIENTS, time(NULL));
/*    while(not_finished){
        fd_set set;
        FD_ZERO(&set);
        FD_SET(local, &set);
        max = local;
        FD_SET(global, &set);
        if(global > max) max = global;
        for(int i = 0; i < CLIENTS; i++){
            if(clients[i].fd > 0){
                FD_SET(clients[i].fd, &set);
                if(clients[i].fd > max) max = clients[i].fd;
            }
        }
        max++;
        struct timeval timer = (struct timeval){
            .tv_sec = 10,
            .tv_usec = 0,
        };
        int ready = select(max, &set, NULL, NULL, &timer);
        handle(ready, ==-1, "Error on io poll.", 0);
        int type = -1;
        int accepted = -1;
        if(ready > 0){
            if(FD_ISSET(local, &set)){
                accepted = accept(local, NULL, NULL);
                type = 0;
                register_sock(time(NULL), type, accepted);
            } else if(FD_ISSET(global, &set)){
                accepted = accept(global, NULL, NULL);
                type = 1;
                register_sock(time(NULL), type, accepted);
            } else {
                for(int i = 0; i < CLIENTS; i++){
                    if(FD_ISSET(clients[i].fd, &set)){
                        received(time(NULL), type, i);
                    }
                }
            }
            
        }
        counter++;
    }*/
    while(not_finished){
        struct pollfd fds[CLIENTS + 2];
        int size = 0;
        fds[0] = (struct pollfd){
            .fd = global,
            .events = POLLIN,
            .revents = 0
        };
        fds[1] = (struct pollfd){
            .fd = local,
            .events = POLLIN,
            .revents = 0
        };
        size = 2;
        for(int i = 0; i < CLIENTS; i++){
            if(clients[i].fd > 0){
                fds[size] = (struct pollfd){
                    .fd = clients[i].fd,
                    .events = POLLIN | POLLRDHUP,
                    .revents = 0  
                };
                size++;
            }
        }
        int added = 0;
        int ready = poll(fds, size, 5000);
        if(ready > 0){
            for(int i = 0; i < 2; i++){
                if(fds[i].revents & POLLIN){
                    int accepted = accept(fds[i].fd , NULL, NULL);
                    int accepted_type = i;
                    fprintf(stderr, "Request for registration of client %d\n", accepted);
                    added += register_sock(time(NULL), accepted_type, accepted);
                }
            }
            int removed = 0;
            for(int i = 2; i < size; i++){
                if(fds[i].revents & POLLIN){
                    received(time(NULL), 0, i-2);
                }
                if(fds[i].revents & POLLRDHUP){
                    fprintf(stderr, "Client %d disconnected\n", clients[i-2].fd);
                    close(fds[i].fd);
                    memcpy(&clients[i-2], &clients[size-3+added], sizeof(client_t));
                    removed++;
                }
            }

            for(int index = size-2+added; index > size-2-removed+added; index--){
                 fprintf(stderr, "Removing client %d from index: %d\n", clients[index].fd, index);
                memset(&clients[index], 0, sizeof(client_t));
            }
        }
        size+=added;
        handle(ready == -1, && (errno != EINTR), "Waiting on poll", 1);
        size = clean_register(time(NULL), size - 2);
    }
    exitfun();
	return 0;
}


int clean_register(time_t current_time, int size){
    for(int i = 0; i < CLIENTS; i++){
        if(clients[i].registred && (clients[i].confirmed == 0)){
            time_t last_access = clients[i].last_access;
            if((current_time - last_access) >= 10){
                fprintf(stderr, "Timeout client %d from index: %d\n", clients[i].fd, i);
                handle(shutdown(clients[i].fd, SHUT_RDWR), == -1, "Can't shutdown client's socket.", 0);
                handle(close(clients[i].fd), == -1, "Can't close socket.", 0);
                size--;
                memcpy(&clients[i], &clients[size], sizeof(client_t));
                memset(&clients[size], 0, sizeof(client_t));
            }
        }
    }
    return size;
}

void received(time_t current_time, int type, int j){
    (void)type;
    msg_t message;
    clients[j].last_access = current_time;
    clients[j].registred = 1;
    clients[j].confirmed = 1;
    int src =  clients[j].fd;
    memset(&message, 0, sizeof(msg_t));
    int readed = -1;
    handle((readed = read(src, &message, sizeof(msg_t))), == -1, "Can't read message", 1);
    char* el = (message.m);
    for(int i = 0; i < BUF_SIZE; i++){
        if((*el) == '\n' || (*el) == '\0'){
            memset(++el, 0, BUF_SIZE - i - 2);
            break;
        }
        el++;
    }
    fprintf(stderr, "%s:%s", message.id, message.m);
    if(readed == 0){
        memset(&clients[j], 0, sizeof(client_t));
    }
    fprintf(stderr, "%s", message.m);
    for(int i = 0; i < CLIENTS; i++){
        if(i != j){
            if((clients[i].fd > 0) && (clients[i].confirmed > 0)){
              //fprintf(stderr, "SENDING... to %d  msg: <%s>: %s \n", clients[i].fd, message.id, message.m);
               handle(write(clients[i].fd, &message, sizeof(msg_t)), == -1, "Can't broadcast message", 0);
            }
        }
    }
}

int register_sock(time_t current_time, int type, int src){
    int i = -1;
    for(i = 0; i < CLIENTS; i++){
        if(clients[i].registred == 0){
            clients[i].last_access = current_time;
            clients[i].type = type;
            clients[i].fd = src;
            clients[i].registred = 1;
            clients[i].confirmed = 0;
            fprintf(stderr, "Registration of client %d at %d\n", src , i);
            break;
        }
    }
    return 1;
}



