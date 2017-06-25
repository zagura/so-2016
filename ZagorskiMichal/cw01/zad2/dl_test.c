/*
	Michal Zagorski
	Systemy Operacyjne 2015/2016
	6 marca 2016
*/

#include <stdio.h>
#include <time.h>
#include <sys/times.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>
#include "../lib/mz_list.h"

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

void time_count(mz_time_t* time_point, int* point){
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
	void* mz_lib_handler = dlopen("../lib/libmz_sharedlib.so", RTLD_NOW);
	if(mz_lib_handler == NULL){
		perror("Wrong opening file");
	}

	mz_time_t* prog_time = calloc(1, sizeof(mz_time_t));
	prog_time->buffer = calloc(1, sizeof(tms_t));
	int i = 0;
	time_count(prog_time, &i);


	Node_t* (*create_list)(void)  = (Node_t* (*)(void)) dlsym(mz_lib_handler, "create_list");
	Node_t* head = create_list();
	Date_t* (*new_date)(int, int, int) = (Date_t* (*)(int, int, int)) dlsym(mz_lib_handler, "new_date");
	Date_t* date = new_date(1995,11,22);
//	typedef void (*init_fun)(Contact_t*, char*, char*, char*, char*, unsigned long long, Date_t) init_fun2;
	void (*init_contact)(Contact_t*, char*, char*, char*, char*, unsigned long long, Date_t) = 
		(void (*)(Contact_t*, char*, char*, char*, char*, unsigned long long, Date_t)) dlsym(mz_lib_handler, "init_contact");
	init_contact(&(head->contact), "Michal", "Zagorski", "mzagorsk@student.agh.edu.pl", "Budryka 123", 123456789, *date);
	Date_t* (*delete_date)(Date_t*) = (Date_t* (*)(Date_t*))dlsym(mz_lib_handler, "delete_date");
	Node_t* contact2 = create_list();
	date = delete_date(date);
	date = new_date(1970,11,22);
	Contact_t con;
	init_contact(&(contact2->contact), "Wociech", "Alski", "abc@wp.pl", "Piekna 123", 145456789, *date);
	init_contact(&con, "Wociech", "Alski", "abc@wp.pl", (char*)NULL , 145456789, *date);
	Node_t* (*push_front)(Node_t*, Node_t*) = 
		(Node_t* (*)(Node_t*, Node_t*))dlsym(mz_lib_handler, "push_front");

	head = push_front(head, contact2);
	time_count(prog_time, &i);

	Node_t* (*search_contact)(Node_t*, Contact_t)  = (Node_t* (*)(Node_t*, Contact_t)) dlsym(mz_lib_handler, "search_contact");
	date =delete_date(date);
	Node_t* con3 = search_contact(head, con);
	printf("Contact2: %p\n ContactS: %p\n", (void*)contact2, (void*)con3);
	time_count(prog_time, &i);

	Node_t* (*sort_list)(Node_t*)  = (Node_t* (*)(Node_t*))dlsym(mz_lib_handler, "sort_list");
	sort_list(head);
	time_count(prog_time, &i);
	con3 = search_contact(head, con);
	printf("Contact2: %p\n ContactS: %p\n", (void*)contact2, (void*)con3);
	Node_t* (*delete_node)(Node_t*) = (Node_t* (*)(Node_t*))dlsym(mz_lib_handler, "delete_node");
	Node_t* prev = delete_node(head->next);
		if(prev == head){
		prev = NULL;
	}
	Node_t* (*delete_list)(Node_t*) = (Node_t* (*)(Node_t*))dlsym(mz_lib_handler, "delete_list");
	head = delete_list(head);
	contact2 = NULL;
	con3 = NULL;

	void (*remove_contact_data)(Contact_t)  = (void (*)(Contact_t)) dlsym(mz_lib_handler, "remove_contact_data");
	remove_contact_data(con);
	remove_contact_data(con);
		for(int i = 0; i < 1000; i++){
			Node_t* node = malloc(sizeof(Node_t)*i);
			if(i%100 == 0){
				int j = 1000 * i;
				while(j--);
			}
			time_count(prog_time, &i);
			free(node);
		}
	time_count(prog_time, &i);
	if( dlclose(mz_lib_handler) == 0){
		printf("Success during closing lib...\n");
	}else{
		printf("%s", dlerror());
	}
	free(prog_time->buffer);
	free(prog_time);
    return 0;
}
