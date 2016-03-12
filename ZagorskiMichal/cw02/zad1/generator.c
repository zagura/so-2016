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

int main(int argc, char** argv){
	int seed = time(NULL);
	srand(seed);
	if(argc != 3){
		printf("Wrong number of arguments\n");
		return 0;
	}
	char* filename = argv[0];
	char* block_size_s = argv[1];
	char* records_count_s = argv[2];

	unsigned long record_size = atol(block_size_s);
	unsigned long long records_count = atoll(records_count_s);
	FILE* file = fopen(filename, "w");
	if(file == NULL){
		perror("Error with file.\n");
		exit(1);
	}
	char* record = calloc(record_size, sizeof(char));
	for(unsigned long long index = 0; index < records_count; index++){

	}
	if(file !=  NULL){
		fclose(file);
	}
	free(record);
	return 0;
}