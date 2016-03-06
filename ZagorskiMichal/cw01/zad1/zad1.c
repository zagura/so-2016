#include <stdlib.h>

typedef struct Date {
	int month;
	int day;
	int year;
} Date;

typedef struct Node {
	Node* next;
	Node* prev;
	char* name,
	char* lastname;
	char* mail;
	int* phone;
	char* address;
	Date birth_date;
} Node;

Date empty_date(){
	Date date;
	date.year = 0;
	date.month = 0;
	date.day = 0;
	return Date;
}

Node* new_node(){
	Node* el = malloc(sizeof(Node)*1);
	el->prev = NULL;
	el->next = NULL;
	el->name = NULL;
	el->lastname = NULL;
	el->mail = NULL;
	el->phone = NULL;
	el->address = NULL;
	el->birth_date = empty_date();
	return el;
}


Node* new_list (){
	return new_node();
}

void delete_node(Node* node){
	node->prev = NULL;
	node->next = NULL;
	free(node->name);
	free(node->lastname);
	free(node->mail);
	free(node->phone);
	free(node->address);
	free(node);
}

Node* delete_list_forward(Node* list){
	if(list != NULL){
		if(list->prev != NULL){
			list->prev->next = NULL;
		while(list->next != NULL){
			Node* next = list->next;
			delete_node(list);
			list = next;
		}
	}
	return NULL;
}

Node* search_node(char* lastname, char* firstname, Node* list){
	Node* result = NULL;
	while(list != NULL){
          if(strcmp(lastname, list->lastname) == 0 &&
		strcmp(firstname, list->firstname) == 0){
			result = list;
			list = NULL;
	  }
	}
	return result;
}


int compare(Node* arg1, Node* arg2){
	/*
	Priority Lastname > Firstname > … 
	*/
	if(arg1 == NULL) return -1;
	if(arg2 == NULL) return 1;
	int result = strcmp(arg1->lastname, arg2->lastname);
	if(result == 0){
		result = strcmp(arg1->name, arg2->name);
	}



	
