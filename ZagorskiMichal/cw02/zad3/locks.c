#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>

struct node{
	struct flock lock;
	struct node* next;
};

typedef struct node node_t;
node_t* lock_list;

int run(char operator, int fd, short* flag);

void close_prog(int file_desc){
	while(lock_list != NULL){
		node_t* tmp = lock_list->next;
	//	free(lock_list->lock);
		free(lock_list);
		lock_list = tmp;
	}
	if(close(file_desc) == -1){
		perror(NULL);
	}
	if(errno)
		exit(EXIT_FAILURE);
	else
		exit(EXIT_SUCCESS);
}
void print_menu(){
	printf("-----------------------------------------\n");
	printf("Menu:\n1. Set read lock at char.\n2. Set write lock at char.\n");
	printf("3. Read all locks.\n");
	printf("4. Free lock.\n");
	printf("5. Read char from file.\n6. Write char to file.\nClose program with other key\n");
	printf("-----------------------------------------\n");
	printf("Choice: ");
}

int main(int argc, char** argv){
	if(argc != 2){
		fprintf(stderr, "Wrong arguments, run program with filename as it's only arg\n");
		exit(EXIT_FAILURE);
	}
	lock_list = NULL;
	char* file = argv[1];
	print_menu();
	char operator = getchar();
	int fd = open(file, O_RDWR);
	short flag = 1;

	while(flag != 0){
		if(run(operator, fd, &flag) == -1){
			perror(file);
			close_prog(fd);
		}
		print_menu();
		operator = getchar();
		if(operator > '6' || operator < '1'){
			flag = 0;
		}
	}

	close_prog(fd);
	exit(EXIT_SUCCESS);
}


 void set_r_lock(int fd, int offset){
	struct flock my_lock;
	my_lock.l_type = F_RDLCK;
	my_lock.l_whence = SEEK_SET;
	off_t off = lseek(fd, offset, SEEK_SET);
	if(off == -1){
		return;
	}
	my_lock.l_start = off;
	off_t len = 1;
	my_lock.l_len = len;
	my_lock.l_pid = getpid();
	if(fcntl(fd, F_SETLK, &my_lock) == -1) return;
	node_t* node = malloc(sizeof(node_t));
	node->lock = my_lock;
	node->next = lock_list;
	lock_list = node;
	printf("Read lock on %d set successfully\n", offset);
}

void set_w_lock(int fd, int offset){
	struct flock my_lock;// = malloc(sizeof(struct flock));
	my_lock.l_type = F_WRLCK;
	my_lock.l_whence = SEEK_SET;
	off_t off = lseek(fd, offset, SEEK_SET);

	if(off == -1){
		return;
	}
	my_lock.l_start = off;
	off_t len = 1;
	my_lock.l_len = len;
	my_lock.l_pid = getpid();
	if(fcntl(fd, F_SETLK, &my_lock) == -1)
		return;
	node_t* node = malloc(sizeof(node_t));
	node->lock = my_lock;
	node->next = lock_list;
	lock_list = node;
	printf("Write lock on %ld set successfully\n", off);
}

void print_lock(struct flock lock){
	if(lock.l_type != F_UNLCK){
		printf("+++++++++++++++++\n");
		printf("Lock is present\n");
		if(lock.l_type == F_WRLCK){
			printf("Type: Write\n");
		}
		if(lock.l_type == F_RDLCK){
			printf("Type: Read\n");
		}
		printf("Offset: %ld\n", lock.l_start);
		printf("Length: %ld\n", lock.l_len);
		printf("Owner's PID: %ld\n", (long)lock.l_pid);
		printf("+++++++++++++++++\n");
	}
}
void read_locks(int fd){
	off_t end = lseek(fd, 0L, SEEK_END);
	off_t off = lseek(fd, 0L, SEEK_SET);
	struct flock lock;
	lock.l_len = 1;
	lock.l_pid = getpid();
	lock.l_whence = SEEK_SET;
	errno = 0;
	if(off == -1 || end == -1) return;
	while(off < end && off > -1){
		lock.l_start = off;
		if(fcntl(fd, F_GETLK, &lock) == -1) return;
		print_lock(lock);
		off = lseek(fd, off+1, SEEK_SET);
	}
	node_t* list = lock_list;
	printf("Locks owned by this process\n");
	while(list != NULL){
		print_lock(list->lock);
		list = list->next;
	}
}

void free_lock(int fd, int offset){
	errno = 0;
	struct flock my_lock;
	my_lock.l_type = F_UNLCK;
	my_lock.l_whence = SEEK_SET;
	off_t off = lseek(fd, offset, SEEK_SET);
	if(off == -1){
		return;
	}
	my_lock.l_start = off;
	off_t len = 1;
	my_lock.l_len = len;
	my_lock.l_pid = getpid();
	if(fcntl(fd, F_SETLK, &my_lock) == -1) return;
	node_t* list = lock_list;
	while(list != NULL){
		if(list->lock.l_start == my_lock.l_start){
			list->lock.l_type = my_lock.l_type;
			break;
		}
		list = list->next;
	}
	printf("Successfully freed the lock on %ld", off);

}

void read_char(int fd, int offset){
	struct flock check_lock;
	check_lock.l_type = F_UNLCK;
	check_lock.l_whence = SEEK_SET;
	off_t off = lseek(fd, offset, SEEK_SET);
	if(off == -1){
		return;
	}
	check_lock.l_start = off;
	off_t len = 1;
	check_lock.l_len = len;
	check_lock.l_pid = getpid();
	if(fcntl(fd, F_GETLK, &check_lock) == -1) return;
	if(check_lock.l_type == F_UNLCK){
		printf("Free record\n");
	}else if(check_lock.l_type == F_WRLCK){
		printf("Write lock is present\n");
	}else if(check_lock.l_type == F_RDLCK){
		printf("Read lock is present");
	}
	char buff;
	read(fd, &buff, sizeof(char));
	printf("Read char: %c\n", buff);
	if(off == -1){
		return;
	}
}

void write_char(int fd, int offset){
	struct flock check_lock;
	check_lock.l_type = F_UNLCK;
	check_lock.l_whence = SEEK_SET;
	off_t off = lseek(fd, offset, SEEK_SET);
	if(off == -1){
		return;
	}
	check_lock.l_start = off;
	off_t len = 1;
	check_lock.l_len = len;
	check_lock.l_pid = getpid();
	errno = 0;
	if(fcntl(fd, F_GETLK, &check_lock) == -1) return;
	if(check_lock.l_type == F_UNLCK){
		printf("Free record\n");
	}else if(check_lock.l_type == F_WRLCK){
		printf("Write lock is present\n");
	}else if(check_lock.l_type == F_RDLCK){
		printf("Read lock is present");
	}
	printf("Input the char: ");
	char buff = getchar();
	getchar();
	write(fd, &buff, sizeof(char));
	printf("Wrote char: %c\n", buff);
	if(off == -1){
		return;
	}
}

int run(char operator, int fd, short* flag){
	int offset = 0;
	switch(operator){
		case '1': case '2': case '4': case '5': case '6':
			printf("Set the offset: ");
			scanf("%d", &offset);
		case '3':
			getchar();			//Empty newline
		default:
			break;
	}
	switch (operator){
		case '1':
			set_r_lock(fd, offset);
			break;
		case '2':
			set_w_lock(fd, offset);
			break;
		case '3':
			read_locks(fd);
			break;		
		case '4':
			free_lock(fd, offset);
			break;		
		case '5':
			read_char(fd, offset);
			break;		
		case '6':
			write_char(fd, offset);
			break;		
		default:
			*flag = 0;
			break;	
	}
	if(errno != 0) return -1;
	return 0;
}

















