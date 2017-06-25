/*
	Michał Zagórski
	Systemy operacyjne 2015/2016
	Zestaw 2 - zadanie 2
	13 marca 2016
*/

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <features.h>

#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 500


typedef struct stat stat_t;
typedef struct dirent dirent_t;
void search_dir(const char* rights, const char* path);

int main(int argc, char** argv){
	if(argc != 3){
		perror("Wrong amount of aruments, should be pathname and right as 4 digits");
		exit(1);
	}
	if(strlen(argv[2]) != 4){
		perror("Wrong format of given rights");
		exit(1);
	}
	search_dir(argv[2], argv[1]);
	return 0;
}



int check_rights(stat_t* buf, const char* rights){
	mode_t mode = buf->st_mode;
	int rights_int = atoi(rights);
	int file_rights = ((mode & (S_ISUID | S_ISGID)) / 512 * 1000) + ((mode & S_IRWXU) / 64 * 100) + ((mode & S_IRWXG) / 8 * 10) + (mode & S_IRWXO);
	if(file_rights == rights_int)
		return 1;
	return 0;
}


void checkfile(dirent_t* dirent, const char* rights, const char* path){
	stat_t buf;

	if(stat(path, &buf) == -1){
		goto stat_error;
	}else{
		if(S_ISDIR(buf.st_mode)){
			search_dir(rights, path);
		}else if(S_ISREG(buf.st_mode)){
			if(check_rights(&buf, rights) == 1){
				off_t size = buf.st_size;
				time_t access = buf.st_atime;
				printf("- Name: %s\n- Size: %lu B\n- Last access: %s\n\n", path, size, ctime(&access));
			}
		}
	}
	return;
	stat_error:
		perror(path);
		exit(1);
}



void search_dir(const char* rights, const char* path){
	errno = 0;
	DIR* root = opendir(path);
	if(root == NULL){
		perror(path);
		return;
	}
	errno = 0;
	dirent_t* file_dirent = readdir(root);
	while(file_dirent != NULL){
		char newpath[1024];
		char* path2 = NULL;
		path2 = strcpy(newpath, path);
		path2 = strcat(path2, "/");
		if(strncmp(file_dirent->d_name, "..", 4) && strncmp(file_dirent->d_name, ".", 3)){
			path2 = strcat(path2, file_dirent->d_name);
			checkfile(file_dirent, rights, path2);
		}
		file_dirent = readdir(root);
	}
	if(file_dirent == NULL && errno != 0){
		perror(path);
	}
	if(root != NULL){
		if(closedir(root) == -1){
			perror(path);
		}
		root = NULL;
	}
}

