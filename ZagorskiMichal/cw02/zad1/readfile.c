#include <stdio.h>
#include <sys/time.h>
#include <sys/resurce.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int compare_records(char* rec1, char* rec2);

void insertion_sort(char* filename, int rec_size, char type);










































int compare_records(char* rec1, char* rec2){
	if(*rec1 > *rec2){
		return 1;
	}else if(*rec1 < *rec2){
		return -1;
	}
	return 0;
}

int lib_sort(const char* pathname, long record_size){
	FILE* file = fopen(pathname, "r+");
	char* record = NULL;
	char* cur_record = NULL;
	if(file == NULL){
		perror("Can't open file %s.", pathname);
	}else{
		while(true){
			int seek_error = fseek(file, record_size, SEEK_CUR);
			if(seek_error == -1){
				goto lib_io_error;
			}else{
				if( !frof(file)){
					long offset = ftell(file);
					cur_record = calloc(record_size, sizeof(char));
					bool insert_flag = true;
					do{
						seek_error = fseek(file, (-1)*record_size, SEEK_CUR);
						if(seek_error == -1 || ftell(file) < 0L){
							goto lib_io_error;
						}else{
							record = calloc(record_size, sizeof(char));
							if(!fread(record, record_size, 1, file)){
								goto lib_io_error;
							}if(fseek(file, record_size, SEEK_CUR) == -1){
								goto lib_io_error;
							}
							if(compare_records(cur_record, record) == 1){
								if(fwrite(record, record_size, sizof(char), file) != record_size){
									goto lib_io_error;
								}
							}else{
								insert_flag = false;
							}
							if(record != NULL){
								free(record);
							}
							record = NULL;
						}
					}while(ftell(file) > 0L && insert_flag);
					if(insert_flag){
						if(fseek(file, (-1)*record_size, SEEK_CUR) == -1){
							goto lib_io_error;
						}
					}
					if(fwrite(cur_record, record_size, sizeof(char), file) != record_size){
						goto lib_io_error;
					}
					if(cur_record != NULL){
						free(cur_record);
					}
					if(fseek(file, offset, SEEK_SET) == -1){
						goto lib_io_error;
					}
				}
			}
		}
	}	
	lib_no_error:
		if(file != NULL){
			fclose(file);
		}
		if(record != NULL){
			free(record);
		}
		if(cur_record != NULL){
			free(cur_record);
		}
		return 0;

	lib_io_error:
		perror("Error during work with file.\n");
		if(file != NULL){
			fclose(file);
		}
		if(record != NULL){
			free(record);
		}
		if(cur_record != NULL){
			free(cur_record);
		}
		return -1;
}


int sys_sort(const char* pathname, long record_size){
	int file = open(pathname, O_RDWR);
	char* record = NULL;
	char* cur_record = NULL;
	if(file == NULL){
		perror("Can't open file %s.", pathname);
	}else{
		while(true){
			int seek_error = fseek(file, record_size, SEEK_CUR);
			if(seek_error == -1){
				goto sys_io_error;
			}else{
				if( !frof(file)){
					long offset = ftell(file);
					cur_record = calloc(record_size, sizeof(char));
					bool insert_flag = true;
					do{
						seek_error = fseek(file, (-1)*record_size, SEEK_CUR);
						if(seek_error == -1 || ftell(file) < 0L){
							goto sys_io_error;
						}else{
							record = calloc(record_size, sizeof(char));
							if(!fread(record, record_size, 1, file)){
								goto sys_io_error;
							}if(fseek(file, record_size, SEEK_CUR) == -1){
								goto sys_io_error;
							}
							if(compare_records(cur_record, record) == 1){
								if(fwrite(record, record_size, sizof(char), file) != record_size){
									goto sys_io_error;
								}
							}else{
								insert_flag = false;
							}
							if(record != NULL){
								free(record);
							}
							record = NULL;
						}
					}while(ftell(file) > 0L && insert_flag);
					if(insert_flag){
						if(fseek(file, (-1)*record_size, SEEK_CUR) == -1){
							goto sys_io_error;
						}
					}
					if(fwrite(cur_record, record_size, sizeof(char), file) != record_size){
						goto sys_io_error;
					}
					if(cur_record != NULL){
						free(cur_record);
					}
					if(fseek(file, offset, SEEK_SET) == -1){
						goto sys_io_error;
					}
				}
			}
		}
	}	
	sys_no_error:
		if(file != NULL){
			fclose(file);
		}
		if(record != NULL){
			free(record);
		}
		if(cur_record != NULL){
			free(cur_record);
		}
		return 0;

	sys_io_error:
		perror("Error during work with file.\n");
		if(file != NULL){
			fclose(file);
		}
		if(record != NULL){
			free(record);
		}
		if(cur_record != NULL){
			free(cur_record);
		}
		return -1;
}
