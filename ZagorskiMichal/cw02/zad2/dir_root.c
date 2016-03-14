/*
	Michał Zagórski
	Systemy operacyjne 2015/2016
	Zestaw 2 - zadanie 2
	14 marca 2016
*/
#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <errno.h>
#include <ftw.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#define PATH_MAX 1024
typedef struct stat stat_t;
typedef struct dirent dirent_t;
int checkfile(const char* path, const struct stat* buf, int flags, struct FTW* ftw);
char* rights = NULL;
int main(int argc, char** argv){
	if(argc != 3){
		perror("Wrong amount of aruments, should be path and rights as 4 digits");
		exit(1);
	}
	if(strlen(argv[2]) != 4){
		perror("Wrong format of given rights");
		exit(1);
	}
	rights = argv[2];
	char path_buffer[PATH_MAX];
	char* abs_path = realpath(argv[1], path_buffer);
	if(abs_path == NULL){
		perror(argv[1]);
		exit(EXIT_FAILURE);
	}
	if(nftw(abs_path, checkfile, 25, FTW_DEPTH | FTW_MOUNT | FTW_PHYS) == -1){
		perror("NFTW");
		exit(EXIT_FAILURE);
	}
	exit(EXIT_SUCCESS);
}

int check_rights(const stat_t* buf){
	mode_t mode = buf->st_mode;
	int rights_int = atoi(rights);
	int file_rights = (
		(mode & (S_ISUID | S_ISGID)) / 512 * 1000)  	//Special rights
		+ ((mode & S_IRWXU) / 64 * 100) 				//User rights
		+ ((mode & S_IRWXG) / 8 * 10) 					//Group
		+ (mode & S_IRWXO);								//Others
	if(file_rights == rights_int)
		return 1;
	return 0;
}


int checkfile(const char* path, const struct stat* buf, int flags, struct FTW* ftw){
	if(S_ISREG(buf->st_mode)){
		if(check_rights(buf) == 1){
			off_t size = buf->st_size;
			time_t access = buf->st_atime;
			printf("- Name: %s\n- Size: %lu B\n- Last access: %s\n\n", path, size, ctime(&access));
		}
	}
	return 0;
}
