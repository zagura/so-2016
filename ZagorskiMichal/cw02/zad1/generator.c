/*
	Systemy operacyjne
	Zestaw 2 ; Zadanie 1
	Michal Zagorski
	9 marca 2016
*/
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv){
	if(argc != 3){
		printf("Wrong number of arguments\n");
		return 0;
	}

	char* filename = argv[0];
	char* block_size_s = argv[1];
	char* records_count_s = argv[2];

	unsigned long block_size = atol(block_size_s);
	unsigned long long records_count = atoq(records_count_s);
	FILE* file = fopen(filename, "w");


	return 0;
}