/*
	Michal Zagorski
	Systemy Operacyjne 2015/2016
	6 marca 2016
*/

#include "../mz_list.h"
#include <stdio.h>
#include <time.h>
#include <sys/times.h>

int main(){
	clock_t cl = clock();
	struct tms* time1 = calloc((size_t)1, sizeof(tms));
	clock_t tim = times(tms);
	printf("%lu \n", time1.tms_utime);
	printf("%lu \n", tim);
	printf("%lu \n", cl);
    return 0;
}