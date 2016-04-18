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


void int_handler(int signo){
    if(signo == SIGINT){
        exit(EXIT_SUCCESS);
    }
}
char* buf = NULL;
int name;
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
int server = 0;

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
/*
void buf_to_msg(msg_t* msg){
    //Buffer size is always 24
    long mtype = (long)buf[7];
    int queue_id = 0;
    for(int i = 3; i >= 0; i--){
        queue_id += (unsigned int)buf[8+i];
        queue_id <<=(i > 0) ? 8 : 0;
    }
    int id = 0;
    for(int i = 3; i >= 0; i--){
        id += (unsigned int)buf[12+i];
        id <<=(i > 0) ? 8 : 0;
    }
    unsigned int number = 0;
    for(int i = 3; i >= 0; i--){
        number += (unsigned int)buf[16+i];
        number <<=(i > 0) ? 8 : 0;
    }
    unsigned int result = 0;
    for(int i = 3; i >= 0; i--){
        result += (unsigned int)buf[12+i];
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
        buf[8+i] = tmp % 256;
        tmp >>= 8;
    }
    tmp = msg->info.id;
    for(int i = 3; i >= 0; i--){
        buf[12+i] = tmp % 256;
        tmp >>= 8;
    }
    unsigned int utmp = msg->info.number;
    for(int i = 3; i >= 0; i--){
        buf[16+i] = utmp % 256;
        utmp >>= 8;
    }
    utmp = msg->info.result;
    for(int i = 3; i >= 0; i--){
        buf[20+i] = utmp % 256;
        utmp >>= 8;
    }
}*/
char* to_name(int num){
    char* n = calloc(6, sizeof(char));
    n[0] = '/';
    for(int i = 1; i < 5; i++){
        n[i] = num % 256;
        num /= 256;
    }
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
    size_t msgsize = 25;

    reset_msg(&msg_client);
    reset_msg(&msg_server);
    msg_client.mtype = Q_RED;
    msg_client.info.id = id;
    
    msg_to_buf(&msg_client);
    handle(mq_send(server, buf, msgsize, Q_RED), == -1, "Can't send ready message", 0);
    //print_message("Ready message", msg_client);
    

    handle(mq_receive(client, buf, msgsize, NULL), == -1, "Can't read number messeage", 0);
    buf_to_msg(&msg_server);
   // print_message("Number message", msg_server);

    reset_msg(&msg_client);

    msg_client.mtype = Q_RES;
    msg_client.info.id = id;
    msg_client.info.number = msg_server.info.number;
    msg_client.info.result = 0;
    msg_client.info.result = is_prime(msg_server.info.number);
    msg_to_buf(&msg_client);
    handle(mq_send(server, buf, msgsize, Q_RES), == -1, "Can't send result message", 0);
    reset_msg(&msg_server);
}

void close_queue(void){
    handle(mq_close(client), == -1, "Closing client's queue", 1);
    msg_t msg_server;
    msg_server.mtype = Q_DEL;
    msg_to_buf(&msg_server);
    handle(mq_send(server, buf, 25, Q_DEL), == -1, "Can't send close queue signal", 1);
    char* tmp_name = to_name(name);   
    handle(mq_unlink(tmp_name), == -1, "Can't unlink POSIX queue", 1);    
    free(tmp_name);
    tmp_name = NULL;
    free(buf);
    buf = NULL;
}


int main(int argc, char** argv){
	handle(argc, != 2, "Wrong number of arguments", 1);
    struct mq_attr attrs;
    attrs.mq_flags = 0;
    attrs.mq_maxmsg = Q_SIZE*2;
    attrs.mq_msgsize = 25*sizeof(char);
    attrs.mq_curmsgs = 0;

    handle((server = mq_open(argv[1], O_WRONLY)), == -1, "Can't open POSIX queue", 1);
    srand(time(NULL));
    name = rand();
    buf = (char*)malloc(25*sizeof(char));
    char* tmp_name = to_name(name);   
    handle((client = mq_open(tmp_name, O_RDWR, 0666, &attrs)), == -1, "Can't open POSIX queue", 1);    

    free(tmp_name);
    tmp_name = NULL;
    handle(atexit(&close_queue), != 0, "Can't set closing command", 1);
    handle(signal(SIGINT, &int_handler), == SIG_ERR, "Can't set signal handling", 0);
    buf[24] = 0;
    msg_t msg_client;
    msg_t msg_server;
    reset_msg(&msg_client);
    reset_msg(&msg_server);
    msg_client.mtype = Q_QID;
    size_t msgsize = 25;
    msg_client.info.queue_id = client;
    msg_to_buf(&msg_client);
    handle(mq_send(server, buf, msgsize, 0), == -1, "Can't send queue id", 1);
    buf_to_msg(&msg_client);
    handle(mq_receive(client, buf, msgsize, NULL), == -1, "Can't get client's id", 1);
    buf_to_msg(&msg_server);
    int id = msg_server.info.id;
   // print_message("received id", msg_server);
    int flag = 0;
    while(!flag){
        count(id);
    }
	return EXIT_SUCCESS;
}