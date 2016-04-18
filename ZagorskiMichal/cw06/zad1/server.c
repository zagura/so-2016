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
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>

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



int server_id = 0;
long* clients = NULL;

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

void reset_msg(msg_t* msg){
    msg->mtype = 0L;
    msg->info.queue_id = -1;
    msg->info.id = -1;
    msg->info.number = 0;
    msg->info.result = 0;
}

void close_queue(void){
    handle(msgctl(server_id, IPC_RMID, NULL), == -1, "Closing client's queue", 1);
    if(!clients){
    	free(clients);
    }
}


int main(int argc, char** argv){
	handle(argc, != 3, "Wrong number of arguments", 1);

	int fd = -1;
	handle((fd = open(argv[1], O_RDWR | O_CREAT, 0666)), == -1 , "Can't acces file", 1);
	close(fd);
	
	key_t queue_key = ftok(argv[1], atoi(argv[2]));
	handle(queue_key, == -1, "Error, when getting key from path", 1);
	server_id = msgget(queue_key, 0666 | IPC_CREAT);
	handle(server_id, == -1, "Error, opening queue for server", 1);
	printf("Key for the server: %d\n", queue_key);

	handle(atexit(&close_queue), != 0, "Can't set closing command", 1);	srand(time(NULL));
	int size = 0;


    msg_t msg_client;
    msg_t msg_server;
    msg_client.mtype = Q_QID;
    size_t msgsize = sizeof(msg_client);

	int counter = 1000;

	while(counter > 0){
		handle(msgrcv(server_id, &msg_client, msgsize, 0, 0), == -1, "Can't read message from queue", 1);
		long msgtype = msg_client.mtype;
		switch(msgtype){
			case Q_QID:{
				int index = size;
				size++;
				handle((clients = (long*)realloc(clients, size*sizeof(long))), == NULL, "Can't allocate memory for array of clients", 0);
				msg_server.mtype = Q_CID;
				msg_server.info.id = index;
				clients[index] = msg_client.info.queue_id;
			//	fprintf(stderr, "value of queue: %ld\n", clients[index]);
			//	fprintf(stderr, "Client msg: %ld %d %d %d %d\n", msg_client.mtype, msg_client.info.queue_id, msg_client.info.id, msg_client.info.number, msg_client.info.result);
			//	fprintf(stderr, "Server msg: %ld %d %d %d %d\n", msg_server.mtype, msg_server.info.queue_id, msg_server.info.id, msg_server.info.number, msg_server.info.result);
				handle(msgsnd(clients[index], &msg_server, msgsize, 0), == -1, "Can't send id message", 0);
				break;
			}
			case Q_RED:{
				msg_server.mtype = Q_NUM;
				msg_server.info.number = rand() + 1;
				int index = msg_client.info.id;
				handle(msgsnd(clients[index], &msg_server, msgsize, 0), == -1, "Can't send next number", 0);
				break;
			}
			case Q_RES:{
				if(msg_client.info.result){
					printf("Liczba pierwsza: %d (klient: %d", msg_client.info.number, msg_client.info.id);
					counter--;
				}
				break;
			}
			default:
				fprintf(stderr, "Unrecognized message in queue\n");
				break;
		}
		fprintf(stderr, "Next loop iteration\n");
	}

	for(int i = 0; i < size; i++){
		int id = clients[i];
		msg_server.mtype = Q_DEL;
		handle(msgsnd(id, &msg_server, msgsize, 0), == -1, "Cant send close message", 0);
	}
	return EXIT_SUCCESS;
}