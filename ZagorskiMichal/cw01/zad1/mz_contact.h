/*
	Michal Zagorski
	Systemy Operacyjne 2015/2016
	2 marca 2016
*/

#ifndef __MZ_CONTACT_H
#define  __MZ_CONTACT_H
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

struct Date {
	short int year;
	unsigned short month;
	unsigned short day;
};

typedef struct Date Date_t;

//extern 
struct Contact {
	char* firstname;
	char* lastname;
	char* mail;
	char* address;
	unsigned long long int phone;
	Date_t birth_date;
};

typedef struct Contact Contact_t;

Contact_t* create_contact();
//static Date_t* create_date();
void init_contact(Contact_t* contact, char* firstname, 
	char* lastname, char* mail, char* address, 
	unsigned long long phone, Date_t birth_date);
void init_date(int year, int month, int day, Date_t* date);
int compare_contacts(Contact_t first, Contact_t second);
Contact_t* remove_contact(Contact_t* contact);
int equal(Contact_t first, Contact_t second);
void remove_contact_data(Contact_t contact);
Date_t* new_date(int year, int month, int day);
Date_t* delete_date(Date_t* date);
#endif
