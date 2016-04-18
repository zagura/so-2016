#define _GNU_SOURCE
#define _XOPEN_SOURCE 500
#define _POSIX_SOURCE 200809L

#include <sys/types.h>
/*#include <sys/ipc.h>
#include <sys/msg.h>*/
#include <mqueue.h>

#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
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
#define Q_SIZE 256

char* buf = NULL;                       
int name = 0;
int clients[Q_SIZE];
void int_handler(int signo){
	if(signo == SIGINT){
		exit(EXIT_SUCCESS);
	}
}
int server_id = 0;

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
void buf_to_msg(msg_t* msg){
    //Buffer size is always 24
    long mtype = (long)buf[7] - 1;
    int queue_id = 0;
    for(int i = 3; i >= 0; i--){
        queue_id += (unsigned int)buf[8+i] - 1;
        queue_id <<=(i > 0) ? 8 : 0;
    }
    int id = 0;
    for(int i = 3; i >= 0; i--){
        id += (unsigned int)buf[12+i] - 1;
        id <<=(i > 0) ? 8 : 0;
    }
    unsigned int number = 0;
    for(int i = 3; i >= 0; i--){
        number += (unsigned int)buf[16+i] - 1;
        number <<=(i > 0) ? 8 : 0;
    }
    unsigned int result = 0;
    for(int i = 3; i >= 0; i--){
        result += (unsigned int)buf[12+i] - 1;
        result <<=(i > 0) ? 8 : 0 ;
    }
    msg->mtype = mtype;
    msg->info.queue_id = queue_id;
    msg->info.id = id;
    msg->info.number = number;
    msg->info.result = result;
    for(int i = 0; i < 24; i++){
        buf[i] = 0;
    }
}

void msg_to_buf(msg_t* msg){
    for(int i = 0; i < 24; i++){
        buf[i] = 0;
    }
    buf[7] = msg->mtype % 256;
    int tmp = msg->info.queue_id;
    for(int i = 3; i >= 0; i--){
        buf[8+i] = tmp % 256 + 1;
        tmp >>= 8;
    }
    tmp = msg->info.id;
    for(int i = 3; i >= 0; i--){
        buf[12+i] = tmp % 256 + 1;
        tmp >>= 8;
    }
    unsigned int utmp = msg->info.number;
    for(int i = 3; i >= 0; i--){
        buf[16+i] = utmp % 256 + 1;
        utmp >>= 8;
    }
    utmp = msg->info.result;
    for(int i = 3; i >= 0; i--){
        buf[20+i] = utmp % 256 + 1;
        utmp >>= 8;
    }
}
char* to_name(int num){
    char* n = calloc(6, sizeof(char));
    n[0] = '/';
    for(int i = 1; i < 5; i++){
        n[i] = num % 256;
        num /= 256;
    }
    return n;
}

void close_queue(void){
	for(int i = 0; i < Q_SIZE; i++){
		if(clients[i] > -1){
			handle(mq_close(clients[i]), == -1, "Closing client's queue", 1);
		}
	}
    char* tmp_name = to_name(name);   
    handle(mq_unlink(tmp_name), == -1, "Can't unlink POSIX queue", 1);    
    free(tmp_name);
    tmp_name = NULL;
    free(buf);
    buf = NULL;
}

void print_message(const char* text, msg_t msg_client){
    fprintf(stderr, "%s: type - %ld Queue - %d Id - %d Number - %d Result - %d\n", 
            text, msg_client.mtype, msg_client.info.queue_id, 
            msg_client.info.id, msg_client.info.number, msg_client.info.result);
}


int main(int argc, char** argv){
	handle(argc, != 2, "Wrong number of arguments", 1);

	for(int i = 0; i < Q_SIZE; i++){
		clients[i] = -1;
	}
    buf = (char*)malloc(25*sizeof(char));
	server_id = mq_open(argv[1], O_RDWR | O_CREAT, 0666);
    struct mq_attr attrs;
    mq_getattr(server_id, &attrs);
    attrs.mq_msgsize = 25*sizeof(char);
    mq_setattr(server_id, &attrs, NULL);
	handle(server_id, == -1, "Error, opening queue for server", 1);

	handle(atexit(&close_queue), != 0, "Can't set closing command", 1);	
	srand(time(NULL));
	handle(signal(SIGINT, &int_handler), == SIG_ERR, "Can't set signal handling", 0);
	int size = 0;


    msg_t msg_client;
    msg_t msg_server;
    size_t msgsize = attrs.mq_msgsize;
	while(1){
		reset_msg(&msg_client);
		//unsigned int type = 0;
		handle(mq_receive(server_id, buf, msgsize, NULL), == -1, "Can't read message from queue", 1);
		buf_to_msg(&msg_client);
		long msgtype = msg_client.mtype;
		switch(msgtype){
			case Q_QID:{
				//print_message("Client's registration", msg_client);
				int index = size;
				size++;
				reset_msg(&msg_server);
				msg_server.mtype = Q_CID;
				msg_server.info.id = index;
				char* tmp_name = to_name(msg_client.info.queue_id);
				msg_to_buf(&msg_server);
				handle((clients[index] = mq_open(tmp_name, O_WRONLY)), == -1, "Can't open client's queue", 0);
				handle(mq_send(clients[index], buf, msgsize, Q_CID), == -1, "Can't send id message", 0);
				//print_message("Server sends id", msg_server);
				break;
			}
			case Q_RED:{
				reset_msg(&msg_server);
				msg_server.mtype = Q_NUM;
				msg_server.info.number = rand() + 1;
				int index = msg_client.info.id;
				msg_to_buf(&msg_server);
				handle(mq_send(clients[index], buf, msgsize, Q_NUM), == -1, "Can't send next number", 0);
				break;
			}
			case Q_RES:{
				if(msg_client.info.result){
					printf("Liczba pierwsza: %d (klient: %d)\n", msg_client.info.number, msg_client.info.id);
				}
				break;
			}
			case Q_DEL:{
				int index = msg_client.info.id;
				handle(mq_close(clients[index]), == -1, "Can't close client's queue", 0);
				clients[index] = -1;
				break;
			}
			default:
				fprintf(stderr, "Unrecognized message in queue\n");
				break;
		}
//		fprintf(stderr, "Next loop iteration\n");
	}
//	fprintf(stderr, "SIZE: %d\n", size);
/*	for(int i = 0; i < size; i++){
		int id = clients[i];
		if(id > -1){
			reset_msg(&msg_server);
			msg_server.mtype = Q_DEL;
			handle(msgsnd(id, &msg_server, msgsize, 0), == -1, "Cant send close message", 0);
		}
	}*/
	return EXIT_SUCCESS;
}