/*
	Michal Zagorski
	Systemy Operacyjne 2015/2016
	6 marca 2016
*/

#include "../lib/mz_list.h"
#include "../lib/mz_contact.h"
#include <stdio.h>
#include <time.h>
#include <sys/times.h>
#include <sys/time.h>
#include <stdlib.h>

typedef struct tms tms_t;
struct mz_time{
	tms_t* buffer;
	clock_t start_real;
	clock_t before_real;
	clock_t before_user;
	clock_t before_system;
	clock_t user;
	clock_t real;
	clock_t sys;
};
typedef struct mz_time mz_time_t;

mz_time_t time_count(mz_time_t* time_point, int* point){
	clock_t time_of_call = times(time_point->buffer);
	if(time_of_call == -1){
		perror("Error during clocking");
	}
	time_of_call = clock();
	time_point->before_real = time_point->real;
	time_point->before_user = time_point->user;
	time_point->before_system = time_point->sys;
	time_point->real = time_of_call;
	time_point->user = time_point->buffer->tms_utime;
	time_point->sys = time_point->buffer->tms_stime;
	printf("Point %d:\n", *point);
	printf("-------------------------------------------\n");
	printf("From begin:\n");
	double sys = (double)(time_point->sys) 
				/ (double)CLOCKS_PER_SEC * 1000.0;
	double user = (double)(time_point->user) 
				/ (double)CLOCKS_PER_SEC * 1000.0;
	double real = (double)(time_point->real) 
				/ (double)CLOCKS_PER_SEC * 1000.0;
	

	printf(" - sys: %lf ms \n - user: %lf ms\n - real: %lf ms\n", sys, user, real);
	printf("-------------------------------------------\n");
	printf("From earlier call of time check:\n");
	sys = (double)(time_point->sys - time_point->before_system) 
				/ (double)CLOCKS_PER_SEC * 1000.0;
	user = (double)(time_point->user - time_point->before_user) 
				/ (double)CLOCKS_PER_SEC * 1000.0;
	real = (double)(time_point->real - time_point->before_real) 
				/ (double)CLOCKS_PER_SEC * 1000.0;
	printf(" - sys: %lf ms\n - user: %lf ms\n - real: %lf ms\n", sys, user, real);
	printf("-------------------------------------------\n");
	(*point)++;
}



int main(){
	mz_time_t* prog_time = calloc(1, sizeof(mz_time_t));
	prog_time->buffer = calloc(1, sizeof(tms_t));
	int i = 0;
	time_count(prog_time, &i);

	Node_t* head = create_list();
	Date_t date;
	date.day = 22;
	date.month = 11;
	date.year = 1995;
	init_contact(&(head->contact), "Michal", "Zagorski", "mzagorsk@student.agh.edu.pl", "Budryka 123", 123456789, date);
	Node_t* contact2 = create_list();
	date.year = 1970;
	init_contact(&(contact2->contact), "Wojciech", "Alski", "abc@wp.pl", "Piekna 123", 145456789, date);
	Contact_t con;	
	init_contact(&con, "Wojciech", "Alski", "abc@wp.pl", (char*)NULL , 145456789, date);
	head = push_front(head, contact2);
	
	time_count(prog_time, &i);
	Node_t* con3 = search_contact(head, con);
	printf("Contact2: %p\n ContactS: %p\n", (void*)contact2, (void*)con3);
	
	time_count(prog_time, &i);
	head = sort_list(head);
	
	time_count(prog_time, &i);
	con3 = search_contact(head, con);
	printf("Contact2: %p\n ContactS: %p\n", (void*)contact2, (void*)con3);
	Node_t* prev = delete_node(head->next);
	if(prev == head){
	 	prev = NULL;
	}
	head = delete_list(head);
	contact2 = NULL;
	con3 = NULL;
	remove_contact_data(con);
	time_count(prog_time, &i);
	free(prog_time->buffer);
	free(prog_time);
    return 0;
}
