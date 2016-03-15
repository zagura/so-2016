/*
	Systemy operacyjne
	Zestaw 2 ; Zadanie 1
	Michal Zagorski
	9 marca 2016
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define MAX_CHAR 256
int main(int argc, char** argv){
	int seed = time(NULL);
	srand(seed);
	printf("%d\n", argc);
	if(argc != 4){
	  	printf("Wrong number of arguments\n");
	  	return 0;
	}
	char* filename = argv[1];
	char* block_size_s = argv[2];
	char* records_count_s = argv[3];
	printf("Args: %s | %s | %s\n", argv[1], argv[2], argv[3]);
	unsigned long record_size = atol(block_size_s);
	unsigned long long records_count = atoll(records_count_s);
	printf("Creating file %s with %llu records of size %lu\n", filename, records_count, record_size);
	FILE* file = fopen(filename, "w");
	if(file == NULL){
		perror("Error with file.\n");
		exit(1);
	}
	char* record = NULL;
	for(unsigned long long index = 0; index < records_count; index++){
		record = calloc(record_size, sizeof(char));
		for(int i = 0; i < record_size; i += sizeof(int)){
			int res = rand();
			for(int a = 0; a < sizeof(int); a++){
				record[i+a] = res % MAX_CHAR;
				res /= MAX_CHAR;
			}
		}
		if(fwrite(record, sizeof(char), record_size, file) != record_size){
			perror("Wrong write bytes");
			goto io_error;
		}
		free(record);
		record = NULL;
	}

	if(file !=  NULL){
		fclose(file);
	}
	if(record != NULL){
		free(record);
	}
	return 0;
	exit(0);
	io_error:
		if(file != NULL){
			fclose(file);
			file = NULL;
		}
		if(record != NULL){
			free(record);
			record = NULL;
		}
		perror("File error.\n");
		exit(1);
}