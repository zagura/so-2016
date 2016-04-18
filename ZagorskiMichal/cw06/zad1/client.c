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

int count(int server, int id){
    msg_t msg_client;
    msg_t msg_server;
    size_t msgsize = sizeof(msg_client);
    msg_client.mtype = Q_RED;
    msg_client.info.id = id;
    handle(msgsnd(server, &msg_client, msgsize, 0), == -1, "Can't send ready message", 0);
    handle(msgrcv(client, &msg_server, msgsize, Q_NUM, 0), == -1, "Can't read number messeage", 0);
    msg_client.mtype = Q_RES;
    msg_client.info.id = id;
    msg_client.info.number = msg_server.info.number;
    msg_client.info.result = is_prime(msg_server.info.number);
    fprintf(stderr, "Number: %d, result %d\n", msg_server.info.number, msg_client.info.result);
    handle(msgsnd(server, &msg_client, msgsize, 0), == -1, "Can't send result message", 0);
    int result = msgrcv(client, &msg_server, msgsize, Q_DEL, IPC_NOWAIT);
    errno = 0;
    return result+1;
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

    msg_t msg_client;
    msg_t msg_server;
    msg_client.mtype = Q_QID;
    size_t msgsize = sizeof(msg_client.info);
    msg_client.info.queue_id = client;
    handle(msgsnd(server_id, &msg_client, msgsize, 0), == -1, "Can't send queue id", 1);
    handle(msgrcv(client, &msg_server, msgsize, Q_CID, 0), == -1, "Can't get client's id", 0);

    int flag = 1;
    while(flag){
        flag = count(server_id, msg_server.info.id);
    }
	return EXIT_SUCCESS;
}