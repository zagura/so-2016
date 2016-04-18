// Client source file
#define _GNU_SOURCE
#define _XOPEN_SOURCE 500
#define _POSIX_SOURCE 200809L

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#define handle(fun, val, text, exited)                                   \
                        if(fun val){                                     \
                            fprintf(stderr, "Line %d\n", __LINE__);      \
                            perror(text "\n");                           \
                            if(exited){                                  \
                                exit(EXIT_FAILURE);                      \
                            }                                            \
                            errno = 0;                                   \
                        }


#define Q_QID 1
#define Q_CID 2
#define Q_RED 3
#define Q_NUM 4
#define Q_RES 5
#define Q_DEL 6

void int_handler(int signo){
    if(signo == SIGINT){
        exit(EXIT_SUCCESS);
    }
}

struct mz_msgbuf{
    long mtype;
    struct msg{
        int queue_id;
        int id;
        unsigned int number;
        unsigned int result;
    } info;
};
typedef struct mz_msgbuf msg_t;

int client = 0;

void reset_msg(msg_t* msg){
    msg->mtype = 0L;
    msg->info.queue_id = -1;
    msg->info.id = -1;
    msg->info.number = 0;
    msg->info.result = 0;
}

void print_message(const char* text, msg_t msg_client){
    fprintf(stderr, "%s: type - %ld Queue - %d Id - %d Number - %d Result - %d\n", 
            text, msg_client.mtype, msg_client.info.queue_id, 
            msg_client.info.id, msg_client.info.number, msg_client.info.result);
}


unsigned int is_prime(unsigned int n){
    if (n < 2) return 0;
    if (n < 4) return 1;
    if ((n % 2 == 0) || (n % 3) == 0) return 0;
    if (n < 25) return 1;
    int divider = 5;
    int half = n / 2 + 1;
    for(divider = 5; divider <= half; divider += 2){
        if( n % divider == 0 ){
            return 1;
        }
    }
    return 0;
}

void count(int server, int id){
    msg_t msg_client;
    msg_t msg_server;
    size_t msgsize = sizeof(msg_client.info);

    reset_msg(&msg_client);
    reset_msg(&msg_server);
    msg_client.mtype = Q_RED;
    msg_client.info.id = id;
    
    if(msgrcv(client, &msg_server, msgsize, Q_DEL, IPC_NOWAIT) == 0){
        errno = 0;
        exit(EXIT_SUCCESS);
    }
    handle(msgsnd(server, &msg_client, msgsize, 0), == -1, "Can't send ready message", 0);
    //print_message("Ready message", msg_client);
    

    handle(msgrcv(client, &msg_server, msgsize, -Q_NUM, 0), == -1, "Can't read number messeage", 0);
    if(msg_server.mtype == Q_DEL){
        exit(EXIT_SUCCESS);
    }
   // print_message("Number message", msg_server);


    reset_msg(&msg_client);

    msg_client.mtype = Q_RES;
    msg_client.info.id = id;
    msg_client.info.number = msg_server.info.number;
    msg_client.info.result = 0;
    msg_client.info.result = is_prime(msg_server.info.number);
    if(msgrcv(client, &msg_server, msgsize, Q_DEL, IPC_NOWAIT) == 0){
        errno = 0;
        exit(EXIT_SUCCESS);
    }
    handle(msgsnd(server, &msg_client, msgsize, 0), == -1, "Can't send result message", 0);
    reset_msg(&msg_server);
        if(msgrcv(client, &msg_server, msgsize, Q_DEL, IPC_NOWAIT) == 0){
        errno = 0;
        exit(EXIT_SUCCESS);
    }
}

void close_queue(void){
    handle(msgctl(client, IPC_RMID, NULL), == -1, "Closing client's queue", 1);
}


int main(int argc, char** argv){
	handle(argc, != 3, "Wrong number of arguments", 1);

	key_t server_key = ftok(argv[1], atoi(argv[2]));
    handle(server_key, == -1, "Error, when getting key from path", 1);
    int server_id = msgget(server_key, 0666);
    handle(server_id, == -1, "Error, opening queue for server", 1);

    client = msgget(IPC_PRIVATE, 0666);

    handle(atexit(&close_queue), != 0, "Can't set closing command", 1);
    handle(signal(SIGINT, &int_handler), == SIG_ERR, "Can't set signal handling", 0);

    msg_t msg_client;
    msg_t msg_server;
    reset_msg(&msg_client);
    reset_msg(&msg_server);
    msg_client.mtype = Q_QID;
    size_t msgsize = sizeof(msg_client.info);
    msg_client.info.queue_id = client;
    handle(msgsnd(server_id, &msg_client, msgsize, 0), == -1, "Can't send queue id", 1);
    handle(msgrcv(client, &msg_server, msgsize, Q_CID, 0), == -1, "Can't get client's id", 1);
    int id = msg_server.info.id;
   // print_message("received id", msg_server);
    int flag = 0;
    while(!flag){
        count(server_id, id);
    }
	return EXIT_SUCCESS;
}