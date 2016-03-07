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
#include <stdlib.h>
typedef struct tms tms_t;


void time_count(tms_t** before, tms_t* start, 
				clock_t* t_before, clock_t t_start, 
				int* point){
	tms_t* current_time = calloc((size_t)1, sizeof(tms_t));
	clock_t time_of_call = times(current_time);
	printf("Point %d:\n", *point);
	printf("-------------------------------------------\n");
	printf("From begin:\n");
	double sys = (double)(current_time->tms_stime - start->tms_stime) 
				/ (double)CLOCKS_PER_SEC * 1000.0;
	double user = (double)(current_time->tms_utime - start->tms_utime) 
				/ (double)CLOCKS_PER_SEC * 1000.0;
	double real = (double)(time_of_call - t_start) 
				/ (double)CLOCKS_PER_SEC * 1000.0;
	

	printf(" - sys: %lf ms \n - user: %lf ms\n - real: %lf ms\n", sys, user, real);
	printf("-------------------------------------------\n");
	printf("From earlier call of time check:\n");
	sys = (double)(current_time->tms_stime - (*before)->tms_stime) 
				/ (double)CLOCKS_PER_SEC * 1000.0;
	user = (double)(current_time->tms_utime - (*before)->tms_utime) 
				/ (double)CLOCKS_PER_SEC * 1000.0;
	real = (double)(time_of_call - (*t_before)) 
				/ (double)CLOCKS_PER_SEC * 1000.0;
	printf(" - sys: %lf ms\n - user: %lf ms\n - real: %lf ms\n", sys, user, real);
	printf("-------------------------------------------\n");
	free((*before));
	*before = current_time;
	*t_before = time_of_call;
	(*point)++;
	return;
}



int main(){
	tms_t* start = calloc((size_t)1, sizeof(tms_t));
	tms_t* before = calloc((size_t)1, sizeof(tms_t));
	clock_t t_start = times(start);
	clock_t t_before = (clock_t)start;
	*before = *start;
	int i = 1;
	time_count(&before, start, &t_before, t_start, &i);
	Node_t* head = create_list();
	Date_t date;
	date.day = 22;
	date.month = 11;
	date.year = 1995;
	init_contact(&(head->contact), "Michal", "Zagorski", "mzagorsk@student.agh.edu.pl", "Budryka 123", 123456789, date);
	Node_t* contact2 = create_list();
	date.year = 1970;
	init_contact(&(contact2->contact), "Wociech", "Alski", "abc@wp.pl", "Piekna 123", 145456789, date);
	Contact_t con;
	init_contact(&con, "Wociech", "Alski", "abc@wp.pl", (char*)NULL , 145456789, date);
	head = push_front(head, contact2);
	time_count(&before, start, &t_before, t_start, &i);
	Node_t* con3 = search_contact(head, con);
	printf("Contact2: %p\n ContactS: %p\n", (void*)contact2, (void*)con3);
	time_count(&before, start, &t_before, t_start, &i);
	sort_list(head);
	time_count(&before, start, &t_before, t_start, &i);
	search_contact(head, con);
	printf("Contact2: %p\n ContactS: %p\n", (void*)contact2, (void*)con3);
	head = delete_list(head);
	contact2 = NULL;
	con3 = NULL;
	remove_contact_data(con);
	time_count(&before, start, &t_before, t_start, &i);
	free(before);
	free(start);
    return 0;
}