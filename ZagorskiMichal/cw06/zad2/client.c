// Client source file
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
#include <signal.h>
#include <time.h>
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



void int_handler(int signo){
    if(signo == SIGINT){
        exit(EXIT_SUCCESS);
    }
}

int name;
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
int client = 0;
int server = 0;
int client_id = -1;
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
/*void buf_to_msg(msg_t* msg){
	memcpy(msg,buf, sizeof(*(msg))); 	
}
void buf2_to_msg(msg_t* msg){
    memcpy(msg,buf2, sizeof(*(msg)));    
}
void msg_to_buf(msg_t* msg){
	memcpy(buf, msg, sizeof(*(msg)));
}*/

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

void count(int id){

    msg_t msg_client;
    msg_t msg_server;
    size_t msgsize = BUF_SIZE;

    reset_msg(&msg_client);
    reset_msg(&msg_server);
    msg_client.mtype = Q_RED;
    msg_client.info.id = id;
    
    char buf_ready[BUF_SIZE];
    memcpy(buf_ready, &msg_client, BUF_SIZE);
    handle(mq_send(server, buf_ready, msgsize, Q_RED), == -1, "Can't send ready message", 0);

    reset_msg(&msg_client);
    
    char buf_id[BUF_SIZE];
    handle(mq_receive(client, buf_id, M_SIZE, NULL), == -1, "Can't read number messeage", 0);
    memcpy(&msg_server, buf_id, BUF_SIZE);
    //buf2_to_msg(&msg_server);
    //print_message("given number ", msg_server);
    reset_msg(&msg_client);

    msg_client.mtype = Q_RES;
    msg_client.info.id = id;
    msg_client.info.number = msg_server.info.number;
    msg_client.info.result = 0;
    msg_client.info.result = is_prime(msg_server.info.number);
    char buf_result[BUF_SIZE];
    memcpy(buf_result, &msg_client, BUF_SIZE);
    handle(mq_send(server, buf_result, msgsize, Q_RES), == -1, "Can't send result message", 0);
    reset_msg(&msg_server);
    reset_msg(&msg_client);
}

void close_queue(void){
    handle(mq_close(client), == -1, "Closing client's queue", 1);
    msg_t msg_server;
    msg_server.mtype = Q_DEL;
    msg_server.info.id = client_id;
    char buf_close[BUF_SIZE];
    memcpy(buf_close, &msg_server, BUF_SIZE);
    handle(mq_send(server, buf_close, 25, Q_DEL), == -1, "Can't send close queue signal", 1);
    char* tmp_name = to_name(name);   
    handle(mq_unlink(tmp_name), == -1, "Can't unlink POSIX queue", 1);    
    free(tmp_name);
    tmp_name = NULL;
}


int main(int argc, char** argv){
	handle(argc, != 2, "Wrong number of arguments", 1);
    struct mq_attr attrs;
    attrs.mq_flags = 0;
    attrs.mq_maxmsg = Q_SIZE;
    attrs.mq_msgsize = 25*sizeof(char);
    attrs.mq_curmsgs = 0;
	argv[1][0] = '/';
    handle((server = mq_open(argv[1], O_WRONLY, 0666, NULL)), == -1, "Can't open server's POSIX queue", 1);
    srand(time(NULL));
    name = rand()+1;
    char* tmp_name = to_name(name);
    handle((client = mq_open(tmp_name, O_RDWR | O_CREAT, 0666, NULL)), == -1, "Can't open client's POSIX queue", 1);
	handle(mq_setattr(client, &attrs, NULL), == -1, "Can't set attributes", 0);    

    free(tmp_name);
    tmp_name = NULL;
    handle(atexit(&close_queue), != 0, "Can't set closing command", 1);
    handle(signal(SIGINT, &int_handler), == SIG_ERR, "Can't set signal handling", 0);
    msg_t msg_client;
    msg_t msg_server;
    reset_msg(&msg_client);
    reset_msg(&msg_server);
    msg_client.mtype = Q_QID;
    size_t msgsize = BUF_SIZE;
    msg_client.info.queue_id = name;
    char buf_name[BUF_SIZE];
    memcpy(buf_name, &msg_client, BUF_SIZE);
    handle(mq_send(server, buf_name, msgsize, 0), == -1, "Can't send queue id", 1);
    char buf_id[BUF_SIZE];
    handle(mq_receive(client, buf_id, M_SIZE, NULL), == -1, "Can't get client's id", 1);
    memcpy(&msg_server, buf_id, BUF_SIZE);
    int id = msg_server.info.id;
    client_id = id;
    int flag = 0;
    while(!flag){
        count(id);
    }
	return EXIT_SUCCESS;
}