/*
	Michał Zagórski
	Systemy Operacyjne 2015/2016
	11 marca 2016
*/
#include <stdio.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

	
int compare_records(char* rec1, char* rec2);

void insertion_sort(char* filename, int rec_size, char type);







int main(int argc, char** argv){

	return 0;
}


































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
		perror("Can't open file.\n");
		return -2;
	}else{
		if(fseek(file, 0, SEEK_END) == -1){
			goto lib_io_error;
		}
		long end_offset = ftell(file);
		if(fseek(file, 0, SEEK_SET) == -1){
			goto lib_io_error;
		}
		long begin_offset = ftell(file);
		long file_size = end_offset - begin_offset;
		long records_count = file_size / record_size;
		if(file_size % record_size == 0){
			perror("Wrong size of file... Not integer count of records");
		}
		for(long i = 1; i < records_count; i++){
			long last_offset = i*record_size + begin_offset;
			if(fseek(file, last_offset, SEEK_SET) == -1){
				goto lib_io_error;
			}//			if(last_offset < end_offset){
			cur_record = calloc(record_size, sizeof(char));
			if(fread(cur_record, record_size, sizeof(char), file) != record_size*sizeof(char)){
				goto lib_io_error;
			}
			bool insert_flag = true;
			for(long offset = (i-1)*record_size; 
				offset >= begin_offset && insert_flag; 
				offset -= record_size){
				record = calloc(record_size, sizeof(char));
				if(fread(record, record_size, sizeof(char), file) != record_size*sizeof(char)){
					goto lib_io_error;
				}if(compare_records(cur_record, record) < 1){
					/* case, when current record <= record in sorted fragment
						moving record from sorted fragment
					*/
					if(fseek(file, record_size, SEEK_CUR) == -1){
						goto lib_io_error;
					}
					if(fwrite(record, record_size, sizeof(char), file) 
						!= record_size*sizeof(char)){
						goto lib_io_error;
					}
					if(offset == begin_offset){
						if(fseek(file, record_size, SEEK_CUR) == -1){
							goto lib_io_error;
						}
						if(fwrite(cur_record, record_size, sizeof(char), file) 
							!= record_size*sizeof(char)){
							goto lib_io_error;
						}
					}
				}else{
					if(fseek(file, record_size, SEEK_CUR) == -1){
						goto lib_io_error;
					}
					if(fwrite(cur_record, record_size, sizeof(char), file) 
						!= record_size*sizeof(char)){
						goto lib_io_error;
					}
					insert_flag = false;
				}
				if(record != NULL){
					free(record);
					record = NULL;
				}
			}
			if(cur_record != NULL){
				free(cur_record);
				cur_record = NULL;
			}
		}
	}	
/*	lib_no_error: */
	if(file != NULL){
		if(fclose(file) != 0){
			goto lib_io_error;
		}
		file = NULL;
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
			file = NULL;
		}
		if(record != NULL){
			free(record);
		}
		if(cur_record != NULL){
			free(cur_record);
		}
		return -1;
}
/*
	sys_sort - sorting file named in pathname, using system calls to communicate with file
*/
int sys_sort(const char* pathname, long record_size){
	int file = open(pathname, O_RDWR);
	char* record = NULL;
	char* cur_record = NULL;
	if(file == -1){
		perror("Can't open file.\n");
		return -2;
	}else{
		off_t begin_offset = lseek(file, 0, SEEK_SET);
		off_t end_offset = lseek(file, 0, SEEK_END);
		if(begin_offset == -1 || end_offset == -1){
			goto sys_io_error;
		}
		off_t file_size = end_offset - begin_offset;
		long records_count = file_size / record_size;
		if(file_size % record_size == 0){
			perror("Wrong size of file... Not integer count of records");
		}
		for(long i = 1; i < records_count; i++){
			long last_offset = i*record_size;

			if(lseek(file, last_offset, SEEK_SET) == -1){
				goto sys_io_error;
			}//			if(last_offset < end_offset){
			cur_record = calloc(record_size, sizeof(char));
			if(read(file, cur_record, record_size*sizeof(char)) != record_size*sizeof(char)){
				goto sys_io_error;
			}
			bool insert_flag = true;
			for(long offset = (i-1)*record_size + begin_offset; 
				offset >= begin_offset && insert_flag; 
				offset -= record_size){
				record = calloc(record_size, sizeof(char));
				if(read(file, record, record_size*sizeof(char)) != record_size*sizeof(char)){
					goto sys_io_error;
				}if(compare_records(cur_record, record) < 1){
					/* case, when current record <= record in sorted fragment
						moving record from sorted fragment
					*/
					if(lseek(file, record_size, SEEK_CUR) == -1){
						goto sys_io_error;
					}
					if(write(file, record, record_size*sizeof(char)) != record_size*sizeof(char)){
						goto sys_io_error;
					}
					if(offset == begin_offset){
						if(lseek(file, record_size, SEEK_CUR) == -1){
							goto sys_io_error;
						}
						if(write(file, cur_record, record_size*sizeof(char)) 
							!= record_size*sizeof(char)){
							goto sys_io_error;
						}
					}
				}else{
					if(lseek(file, record_size, SEEK_CUR) == -1){
						goto sys_io_error;
					}
					if(write(file, cur_record, record_size*sizeof(char)) 
						!= record_size*sizeof(char)){
						goto sys_io_error;
					}
					insert_flag = false;
				}
				if(record != NULL){
					free(record);
					record = NULL;
				}
			}
			if(cur_record != NULL){
				free(cur_record);
				cur_record = NULL;
			}
		}
	}	
/*	sys_no_error: */
	if(file != 0){
		if(close(file) != 0){
			goto sys_io_error;
		}
		file = 0;
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
		if(file != 0){
			close(file);
			file = 0;
		}
		if(record != NULL){
			free(record);
		}
		if(cur_record != NULL){
			free(cur_record);
		}
		return -1;
}
