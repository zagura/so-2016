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


#define LISTEN 50

char path[BUF_SIZE];
char* full_command = NULL;
int port = 0;
int fd = -1;
struct connection{
    char arguments[1024];
    int interval_s;
    char command[10];
}__attribute__((packed));

int alarmtime = 0;
typedef struct connection connection_t;
struct sockaddr_in net_addr;
socklen_t net_len = sizeof(net_addr);
int global;
int not_finished = 1;

void exitfun(void){
    not_finished = 0;
    if(global != 0){
        //handle(shutdown(global, SHUT_RDWR), == -1, "Can't shudown socket.", 0);
        handle(close(global), == -1, "Can't close socket fd", 0);
        global = 0;
    }
    if(full_command != NULL){
        free(full_command);
        full_command = NULL;
    }
}

void signaled(int signo){
    (void)signo;
    exitfun();
}

void alarmed(int signo){
    errno = 0;
    if(signo == SIGALRM){
        if(full_command == NULL){
            exit(0);
        }
        FILE* p_file = popen(full_command, "r");
        handle(p_file, == NULL, "Can't open process", 0);
        char* line = NULL;
        char buffer[4096];
        size_t size;
        while(getline(&line, &size, p_file) != -1){
            int len = strlen(line);
            if(line[len-1] == '\n'){
            line[len-1] = '\0';
            }
            memset(buffer, 0, sizeof(buffer));
            strncpy(buffer, line, len);
            free(line);
            line = NULL;
            size = 0;
            struct pollfd fds[1];
            fds[0] = (struct pollfd){
                .fd = STDOUT_FILENO,
                .events = POLLIN | POLLRDHUP,
                .revents = 0
            };
            int ready = poll(fds, 1, 2);
            if(ready > 0){
                if(fds[0].revents & POLLRDHUP){
                    handle(close(STDOUT_FILENO), == -1, "Can't close socket.", 0);
                    _exit(1);
                }
            }
            handle(write(STDOUT_FILENO, buffer, sizeof(buffer)), == -1, "Can't pipe the output.", 0);
        }
        if(line != NULL){
            free(line);
            line = NULL;
            size = 0;
        }
        if(alarmtime > 0){
            int res = alarm(alarmtime);
            handle(0, == 0, "signal branch", 0);
            handle(res, == 0, "Send alarm signal", 0);
        }else{
             sleep(1);
             handle(shutdown(STDOUT_FILENO, SHUT_RDWR), == -1, "Can't shutdown client's socket.", 0);
             handle(close(STDOUT_FILENO), == -1, "Can't close socket.", 0);
        }
    }
}
int parse_command(connection_t* cmd);
void* fork_service(void* args){
    int fd = *(int*)args;
    struct pollfd mz_poll = (struct pollfd){
        .fd = fd,
        .events = POLLIN | POLLRDHUP,
        .revents = 0
    };
    connection_t params;
    memset(&params, 0, sizeof(connection_t));
    int ready = poll(&mz_poll, 1, 10000);
    if(ready > 0){
        handle(read(fd, &params, sizeof(connection_t)), == -1, "can't receive configuration for service", 0);
    }
    else pthread_exit(0);
    pid_t child_pid; 
    handle((child_pid = fork()), == -1, "Can;t fork new process", 0);
    if(child_pid > 0){
        int status;
        wait(&status);
        pthread_exit(0);
    }else if(child_pid == 0){
        //handle(0, == 0, "DEBUG", 0);
        handle(dup2(fd, STDOUT_FILENO), == -1, "Can't write to socket because of dup2 function", 0);
        struct sigaction sa;
        sa.sa_handler = &alarmed;
        sa.sa_flags = 0;
        if(sigfillset(&(sa.sa_mask)) == - 1){
            perror("Mask set");
        }
        sigdelset(&(sa.sa_mask), SIGALRM);
        int pc = parse_command(&params);
        if(pc == -1){
             handle(shutdown(STDOUT_FILENO, SHUT_RDWR), == -1, "Can't shutdown client's socket.", 0);
             handle(close(STDOUT_FILENO), == -1, "Can't close socket.", 0);
             _exit(1);
        }
        handle(sigaction(SIGALRM, &sa, NULL), == -1, "Can't set signal handler", 0);
        alarmed(SIGALRM);
        for(;;){
            pause();
        }
    }
    pthread_exit(0);
}


int main(int argc, char** argv){
    handle(argc, != 2, "Wrong amount of arguments: program <port>", 1);
    port = atoi(argv[1]);

    memset(&net_addr, 0, sizeof(net_addr));

    net_addr.sin_family = AF_INET;
    net_addr.sin_port = htons(port);
    net_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    handle((global = socket(AF_INET, SOCK_STREAM,IPPROTO_TCP)), == -1, "Can't create socket.",1);
    handle(setsockopt(global, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)), == -1, "Can't set reusable address.", 0);
    handle(bind(global, (struct sockaddr*)&net_addr, net_len), == -1, "Can't bind global address.", 1);
    signal(SIGINT, &signaled);
    atexit(&exitfun);
    handle(listen(global, LISTEN), == -1, "Error in listen operation", 1);
    //daemon(0,0);
    not_finished = 10;
    int err_fd = open("/var/log/service_manager", 0744 | O_CREAT | O_RDWR);
    dup2(err_fd, STDERR_FILENO);
    while(not_finished){
        struct pollfd fds[1];
        int size = 0;
        fds[0] = (struct pollfd){
            .fd = global,
            .events = POLLIN,
            .revents = 0
        };
        size = 1;
        int ready = poll(fds, size, 5000);
        if(ready > 0){
            for(int i = 0; i < 1; i++){
                if(fds[i].revents & POLLRDHUP){
                    not_finished = 0;
                }
                if(fds[i].revents & POLLIN){
                    int accepted = accept(fds[i].fd , NULL, NULL);
                    pthread_t thread;
                    handle((errno = pthread_create(&thread, NULL, &fork_service, &accepted)), > 0, "Can't create thread", 1);
                    handle((errno = pthread_detach(thread)), >0, "Can't detach thread", 0)
                }

            }
        }
        handle(ready == -1, && (errno != EINTR), "Waiting on poll", 1);
        not_finished--;
    }
    exitfun(); // Z zabawy się nie wychodzi :D
    return 0;
}


int parse_command(connection_t* cmd){
    int len = strlen(cmd->command);
    char w_command[] ="w";
    char ifconfig[] = "ifconfig";
    char ps[] = "ps";
    char df[] = "df";
    char inotifywait[] = "inotifywait";
    char vmstat[] = "vmstat";
    int args_len = strlen(cmd->arguments);
    int flag = 0;
    if(strncmp(w_command, cmd->command, sizeof(cmd->command)) == 0){
        flag = 1;
    }else if(strncmp(ifconfig, cmd->command, sizeof(cmd->command)) == 0){
        flag = 2;
    }else if(strncmp(ps, cmd->command, sizeof(cmd->command)) == 0){
        flag = 4;
    }else if(strncmp(df, cmd->command, sizeof(cmd->command)) == 0){
        flag = 8;
    }else if(strncmp(inotifywait, cmd->command, sizeof(cmd->command)) == 0){
        flag = 16;
    }else if(strncmp(vmstat, cmd->command, sizeof(cmd->command)) == 0){
        flag = 32;
    }
    if(flag & 020){
        full_command = (char*)calloc(len + args_len + 2, sizeof(char));
    }else{
        full_command = (char*)calloc(len + 1, sizeof(char));
    }
    strncpy(full_command, cmd->command, len);
    if(full_command[len -1] == '\n'){
        full_command[(len-1)] = ' ';
    }
    if(flag == 0) return -1;
    if(flag & 020){
        full_command[len] = ' ';
        strncpy(&(full_command[len]), cmd->arguments, args_len);
        len = len + args_len;
    }
    if(full_command[len-1] == ' ' || full_command[len-1] == '\n'){
        full_command[len-1] = '\0';
    }
    if(flag & 045){
        alarmtime = cmd->interval_s;
    }
    return 0;
}
