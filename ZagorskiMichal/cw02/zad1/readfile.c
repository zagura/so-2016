/*
	Michał Zagórski
	Systemy Operacyjne 2015/2016
	11 marca 2016
*/
#include <stdio.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/resource.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#define RES_FILE "wyniki.txt"

int compare_records(char* rec1, char* rec2);
int lib_sort(const char* pathname, long record_size);
int sys_sort(const char* pathname, long record_size);


int main(int argc, char** argv){
	if(argc != 4){
	  	printf("Wrong number of arguments\n");
	  	return 0;
	}
	
	printf("Args:\n- Filename: %s\n- Record's size: %s\n- Type: %s\n", argv[1], argv[2], argv[3]);
	
	long record_size = atol(argv[2]);
	char* lib = "lib";
	char* sys = "sys";
	struct rusage mz_time;
	
	clock_t begin_time = clock();
	if(getrusage(RUSAGE_SELF, &mz_time) == -1){
		goto error;
	}
	double begin_time_sys = 
		((double)mz_time.ru_stime.tv_sec * 1000.0 )
		+ ((double)mz_time.ru_stime.tv_usec / 1000.0);
	
	double begin_time_user = 
			((double)mz_time.ru_utime.tv_sec * 1000.0 )
		+ ((double)mz_time.ru_utime.tv_usec / 1000.0);
	
	if(strncmp(lib, argv[3], 3) == 0){
		if(lib_sort(argv[1], record_size) != 0)
			goto error;
	}else if(strncmp(sys, argv[3], 3) == 0){
		if(sys_sort(argv[1], record_size) != 0)
			goto error;
	}

	clock_t end_time = clock();
	if(getrusage(RUSAGE_SELF, &mz_time) == -1){
		goto error;
	}
	
	double end_time_sys = 
		((double)mz_time.ru_stime.tv_sec * 1000.0 )
		+ ((double)mz_time.ru_stime.tv_usec / 1000.0);
	
	double end_time_user = 
			((double)mz_time.ru_utime.tv_sec * 1000.0 )
		+ ((double)mz_time.ru_utime.tv_usec / 1000.0);
	
	double real_time = (double)(end_time - begin_time) / ((double) CLOCKS_PER_SEC / 1000.0);
	double user_time = end_time_user - begin_time_user;
	double sys_time = end_time_sys - begin_time_sys;

	FILE* results = fopen(RES_FILE, "a+");
	if(results == NULL)
		goto error;
	fprintf(results, "\n\n-------------------------------------------------------------------------\n-------------------------------------------------------------------------\n");
	fprintf(results, "Args:\n- Filename: %s\n- Record's size: %s\n- Type: %s\n", argv[1], argv[2], argv[3]);
	fprintf(results, "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	fprintf(results, "Times: \n- real: %lf ms\n- user: %lf ms\n- sys: %lf ms\n", real_time, user_time, sys_time);
	fprintf(results, "-------------------------------------------------------------------------\n");
	fclose(results);
	return 0;
	error:
		perror("Error: ");
		exit(1);
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
		if(file_size % record_size != 0){
			perror("Wrong size of file... Not integer count of records");
		}
		for(long i = 1; i < records_count; i++){
			long last_offset = i*record_size + begin_offset;
			if(fseek(file, last_offset, SEEK_SET) == -1){
				goto lib_io_error;
			}//			if(last_offset < end_offset){
			cur_record = calloc(record_size, sizeof(char));
			if(fread(cur_record, sizeof(char), record_size, file) != record_size){
				goto lib_io_error;
			}
			bool insert_flag = true;
			for(long offset = last_offset - record_size; 
				(offset >= begin_offset) && insert_flag; 
				offset -= record_size){
				if(fseek(file, offset, SEEK_SET) == -1)
					goto lib_io_error;
				record = calloc(record_size, sizeof(char));
				if(fread(record, sizeof(char), record_size, file) != record_size){
					goto lib_io_error;
				}
				if(compare_records(cur_record, record) < 1){
					/* case, when current record <= record in sorted fragment
						moving record from sorted fragment
					*/
					// if(fseek(file, record_size, SEEK_CUR) == -1){		
						// What if fread() moves the record ??
					// 	goto lib_io_error;
					// }
					if(fwrite(record, sizeof(char), record_size, file) 
						!= record_size){
						goto lib_io_error;
					}
					if(offset == begin_offset){
						 if(fseek(file, offset, SEEK_SET) == -1){
						 	goto lib_io_error;
						 }
						if(fwrite(cur_record, sizeof(char), record_size, file) 
							!= record_size){
							goto lib_io_error;
						}
					}
				}else{
					// if(fseek(file, record_size, SEEK_CUR) == -1){
					// 	goto lib_io_error;
					// }
					if(fwrite(cur_record, sizeof(char), record_size, file) 
						!= record_size){
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
		if(file_size % record_size != 0){
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
			for(long offset = last_offset - record_size; 
				offset >= begin_offset && insert_flag; 
				offset -= record_size){
				if(lseek(file, offset, SEEK_SET) == -1){
					goto sys_io_error;
				}
				record = calloc(record_size, sizeof(char));
				if(read(file, record, record_size*sizeof(char)) != record_size*sizeof(char)){
					goto sys_io_error;
				}if(compare_records(cur_record, record) < 1){
					/* case, when current record <= record in sorted fragment
						moving record from sorted fragment
					*/
			/*		
					if(lseek(file, record_size, SEEK_CUR) == -1){
						goto sys_io_error;
					}
			*/
					if(write(file, record, record_size*sizeof(char)) != record_size*sizeof(char)){
						goto sys_io_error;
					}
					if(offset == begin_offset){
						if(lseek(file, offset, SEEK_SET) == -1){
							goto sys_io_error;
						}

						if(write(file, cur_record, record_size*sizeof(char)) 
							!= record_size*sizeof(char)){
							goto sys_io_error;
						}
					}
				}else{
				/*
					if(lseek(file, record_size, SEEK_CUR) == -1){
						goto sys_io_error;
					}
				*/
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
