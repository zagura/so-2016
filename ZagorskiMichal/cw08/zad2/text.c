#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#define RECORD 1024

int main(){
	FILE* file = fopen("Textfile.txt", "rw+");
	if(file == NULL) return EXIT_FAILURE;
	int id = 0;
	fseek(file, 0L, SEEK_END);
	long size = ftell(file);
	rewind(file);
	fprintf(stdout, "File size: %ld\n", size);
	while(ftell(file) < size){
		fwrite(&id, sizeof(int), 1, file);
		fseek(file, RECORD-sizeof(int), SEEK_CUR);
		id++;
	}
		fprintf(stdout, "File end id: %d\n", id);
	fclose(file);
	return 0;
}