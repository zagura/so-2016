#define _GNU_SOURCE
#define _XOPEN_SOURCE 500
#define _POSIX_SOURCE 200809L

#include <sys/types.h>
#include <mqueue.h>

#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>

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
#define Q_SIZE 10
#define M_SIZE 8192
char* tmp_name;                     
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
}__attribute__ ((packed));

typedef struct mz_msgbuf msg_t;
#define BUF_SIZE (sizeof(struct mz_msgbuf))

void reset_msg(msg_t* msg){
    msg->mtype = 0L;
    msg->info.queue_id = -1;
    msg->info.id = -1;
    msg->info.number = 0;
    msg->info.result = 0;
}


char* to_name(int num){
    char* n = calloc(6, sizeof(char));
    n[0] = '/';
    for(int i = 1; i < 5; i++){
        n[i] = num % 255 + 1;
        num /= 256;
    }
    n[5] = '\0';
    return n;
}

void close_queue(void){
	for(int i = 0; i < Q_SIZE; i++){
		if(clients[i] > -1){
			handle(mq_close(clients[i]), == -1, "Closing client's queue", 1);
		}
	} 
    handle(mq_unlink(tmp_name), == -1, "Can't unlink POSIX queue", 1);    
    tmp_name = NULL;

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
    struct mq_attr attrs;
    attrs.mq_flags = 0;
    attrs.mq_maxmsg = Q_SIZE;
    attrs.mq_msgsize = M_SIZE;
    attrs.mq_curmsgs = 0;
	argv[1][0] = '/';
	server_id = mq_open(argv[1], O_RDWR | O_CREAT, 0666, NULL);
	handle(server_id, == -1, "Error, opening queue for server", 1);
	handle(mq_setattr(server_id, &attrs, NULL), == -1, "Can't set attributes", 0);
	tmp_name = argv[1];
	handle(atexit(&close_queue), != 0, "Can't set closing command", 1);	
	srand(time(NULL));
	handle(signal(SIGINT, &int_handler), == SIG_ERR, "Can't set signal handling", 0);
	int size = 0;


    msg_t msg_client;
    msg_t msg_server;
	while(1){
		reset_msg(&msg_client);
		char buf2[BUF_SIZE];
		handle(mq_receive(server_id, buf2, M_SIZE, NULL), == -1, "Can't read message from queue", 1);
		memcpy(&msg_client, buf2, BUF_SIZE);
		//print_message("Server received ", msg_client);
		long msgtype = msg_client.mtype;
		switch(msgtype){
			case Q_QID:{
				int index = size;
				size++;
				reset_msg(&msg_server);
				msg_server.mtype = Q_CID;
				msg_server.info.id = index;
				char* tmp_name2 = to_name(msg_client.info.queue_id);
				char buf_name[BUF_SIZE];
				memcpy(buf_name, &msg_server, BUF_SIZE);
				handle((clients[index] = mq_open(tmp_name2, O_WRONLY, 0666, NULL)), == -1, "Can't open client's queue", 0);
				handle(mq_send(clients[index], buf_name, BUF_SIZE, Q_CID), == -1, "Can't send id message", 0);
				free(tmp_name2);
				break;
			}
			case Q_RED:{
				reset_msg(&msg_server);
				msg_server.mtype = Q_NUM;
				msg_server.info.number = rand() + 1;
				int index = msg_client.info.id;
				char buf_num[BUF_SIZE];
				memcpy(buf_num, &msg_server, BUF_SIZE);
				handle(mq_send(clients[index], buf_num, BUF_SIZE, Q_NUM), == -1, "Can't send next number", 0);
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

	}
	return EXIT_SUCCESS;
}