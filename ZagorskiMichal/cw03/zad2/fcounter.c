#define _GNU_SOURCE 1
#define _XOPEN_SOURCE 700
#define _POSIX_C_SOURCE 200809L


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <errno.h>
#include <time.h>
#include <sched.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#define STACK_SIZE (1024*32)

typedef struct dirent dirent_t;


int main(int argc, char* argv[], char* envp[]){
	char* path = getenv("PATH_TO_BROWSE");
	char program_path[513];
	char* program = realpath("./fcounter", program_path);
	int flag = 0;
	for(int i = 1; i < argc; i++){
		if(argv[i][0] != '-'){
			if(path == NULL){
				path = argv[i];
			}
		}
		else if(strncmp(argv[i], "-w", 2) == 0){
			flag += 2;
		}else if(strncmp(argv[i], "-v", 2) == 0){
			flag += 4;
		}
	}
	if(path == NULL){
		char p[] = ".";
		path = p;
	}
	if(path == NULL){
		perror("No directory to search");
		exit(0);
	}
	errno = 0;
/*	char* stack = (char*)malloc(STACK_SIZE);
	if(stack == NULL){
		perror(path);
	}
	char* top_stack = stack + STACK_SIZE;*/
	int files = 0;
	int children = 0;
	DIR* root = opendir(path);
	if(root == NULL){
		perror(path);
		return 0;
	}
	char* extention = getenv("EXT_TO_BROWSE");
	errno = 0;
	dirent_t* file_dirent = readdir(root);
	struct stat file_data;
	while(file_dirent != NULL){
		char newpath[1024];
		char* path2 = NULL;
		path2 = strcpy(newpath, path);
		path2 = strcat(path2, "/");
		if(strncmp(file_dirent->d_name, "..", 4) && strncmp(file_dirent->d_name, ".", 3)){
			path2 = strcat(path2, file_dirent->d_name);
			if(stat(path2, &file_data) == -1){
				perror(path2);
				perror("Line 68");
			}
			if(S_ISDIR(file_data.st_mode)){
				if(setenv("PATH_TO_BROWSE", path2, 1) == -1 ||
					execve(program, argv, envp) == -1){
					perror("exec");
				}
			}else{
				if(extention != NULL){
					char* name = file_dirent->d_name;
					int len = strlen(name);
					char* sufix = name + len - strlen(extention);
					if(strncmp(sufix, extention, strlen(extention)) == 0) files++;
				}
				else{
					files++;
				}
			}
		}
		file_dirent = readdir(root);
		perror("Line 94");
		perror(file_dirent->d_name);
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
	int files_add = 0;
	int my_files = files;
if(flag & 2){
	sleep(15);
}
	for(int i = 0; i <children; i++){
		if(wait(&files_add) == -1){
			perror(NULL);
		}
		errno = 0;
		if(files_add > 0) files += files_add;
	}
if(flag & 4){
	printf("\nPath: %s", path);
	printf("\nFiles in this directory: %d\n", my_files);
}
	printf("All files in direcories with root in . : %d\n", files);
	exit(files);
	return 0;
}