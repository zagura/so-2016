/*
	Michal Zagorski
	Systemy Operacyjne 2015/2016
	2 marca 2016
*/

#include "mz_contact.h"

Contact_t* create_contact(){
	Contact_t* contact = malloc(sizeof(Contact_t)*1);
	if(contact != NULL) {
		contact->firstname = NULL;
		contact->lastname = NULL;
		contact->address = NULL;
		contact->mail = NULL;
		contact->phone = 0;
		init_date(0,0,0,&(contact->birth_date));
	}
	else{
		/*fprintf(stderr, ) */
	}
	return contact;
}


/* static Date_t* create_date(){
	Date_t* date = malloc(sizeof(Date_t)*1);
	if(date == NULL){
		char* err_msg = "Error in malloc function";
		perror(err_msg);
	}
	return date;
}
*/


void init_contact(Contact_t* contact, char* firstname, char* lastname, 
					char* mail, char* address, 
					unsigned long long phone, Date_t birth_date){
	contact->firstname = firstname;
	contact->lastname = lastname;
	contact->mail = mail;
	contact->address = address;
	contact->phone = phone;
	contact->birth_date = birth_date;
}


void init_date(int year, int month, int day, Date_t* date){
	if(date == NULL){
		return;
	}
	date->year = year;
	date->month = month;
	date->day = day;
}


int compare_contacts(Contact_t* first, Contact_t* second){
	/*
		Priority Lastname > Firstname
	*/
	
	if(first == NULL){
		if(second == NULL){
			perror("Given two empty contacts to comapre");
			return 0;
		}
		return 1;
	}else if(second == NULL){
		return -1;
	}
	int compare_result = 0;
	compare_result = strcmp(first->lastname, second->lastname);
	if(compare_result == 0){
		compare_result = strcmp(first->firstname, second->firstname);
	}
	return compare_result;
}

Contact_t* remove_contact(Contact_t* contact){
	free(contact->firstname);
	free(contact->lastname);
	free(contact->address);
	free(contact->mail);
	free(contact);
	return NULL;
}

int date_cmp(Date_t first, Date_t second){
	int equality = 0;
	/*
	
		Result of function gives full information about the comparations
	*/
	equality += ((first.year < second.year) ? 
		-9 : (first.year > second.year) ? 9 : 0 );
	
	equality += ((first.month < second.month) ? 
		-3 : (first.month > second.month) ? 3 : 0 );
	
	equality += ((first.day < second.day) ? 
		-1 : (first.day > second.day) ? 1 : 0 );
	return equality;
}	
int equal(Contact_t first, Contact_t second){
	/*
		equality == 0 => True
		else => False
	*/
	int equality = 0;
	Date_t empty_date;
	init_date(0,0,0,&empty_date);
	int eq_firstname = 0;
	if(first.firstname != NULL && second.firstname != NULL){
		eq_firstname = strcmp(first.firstname, second.firstname);
		equality += (eq_firstname != 0 ? 1 : 0);
	}
	int eq_lastname = 0;
	if(first.lastname != NULL && second.lastname != NULL){
		eq_lastname = strcmp(first.lastname, second.lastname);
		equality += (eq_lastname != 0 ? 1 : 0);
	}
	int eq_address = 0;
	if(first.address != NULL && second.address != NULL){
		eq_address = strcmp(first.address, second.address);
		equality += (eq_address != 0 ? 1 : 0);
	}
	int eq_mail = 0;
	if(first.mail != NULL && second.mail != NULL){
		eq_mail = strcmp(first.mail, second.mail);
		equality += (eq_mail != 0 ? 1 : 0);
	}
	int eq_date = 0;
	if(date_cmp(first.birth_date, empty_date) != 0 && 
		date_cmp(second.birth_date, empty_date) != 0){
		
		eq_date = date_cmp(first.birth_date, second.birth_date);
		equality += (eq_date != 0 ? 1 : 0);
	}
	int eq_phone = 0;
	if(first.phone != 0 && second.phone != 0){
		eq_phone = (first.phone == second.phone ? 0 : 1);
		equality += eq_phone;
	}
	return equality;
}

void remove_contact_data(Contact_t contact){
	free(contact.firstname);
	free(contact.lastname);
	free(contact.address);
	free(contact.mail);
}