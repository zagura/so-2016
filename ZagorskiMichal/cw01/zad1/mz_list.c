/*
	Michal Zagorski
	Systemy Operacyjne 2015/2016
	2 marca 2016
*/

#include "mz_list.h"


struct Node {
	struct Contact contact;
	struct Node* next;
	struct Node* prev;
};

void delete_node(Node_t* head){
	remove_contact_data(head->contact);
	head->next = NULL;
	head->prev = NULL;
	free(head);
}

Node_t* create_list (){
	Node_t* node = malloc(sizeof(Node_t)*1);
	Date_t empty_date;
	init_date(0,0,0,&(empty_date));
	init_contact(&(node->contact),NULL,NULL, NULL, NULL, 0, empty_date);
	node->next = NULL;
	node->prev = NULL;
	return node;
}

Node_t* delete_list (Node_t* head){
	while(head!=NULL){
		Node_t* next = head->next;
		delete_node(head);
		head = next;
	}
	return head;
}

Node_t* push_back (Node_t* head, Node_t* inserted){
	if(head == NULL){
		head = inserted;
	}
	else{
		Node_t* node = head;
		while(node->next != NULL){
			node = node->next;
		}
		add_next(node, inserted);
	}
	return head;

}
void add_next (Node_t* node, Node_t* inserted){
	if(node == NULL){
		perror("No node to be inserted in");
	}else if(inserted == NULL){
		perror("Inserting null node");
	}else{
		inserted->next = node->next;
		node->next = inserted;
		inserted->prev = node;		
	}
}

void add_prev (Node_t* node, Node_t* inserted){
	if(node == NULL){
		perror("No node to be inserted in");
	}else if(inserted == NULL){
		perror("Inserting null node");
	}else{
		inserted->prev = node->prev;
		node->prev = inserted;
		inserted->next = node;		
	}
}

Node_t* push_front(Node_t* node, Node_t* inserted){
	/* Returns a head of a newly created list */
	if(node != NULL){
		while(node->prev != NULL){
			node = node->prev;
		}
		add_prev(node, inserted);
	}
	return inserted;
}

Node_t* sort_list(Node_t* head);



Node_t* search_contact(Node_t* head, Contact_t contact){
	Node_t* found = NULL;
	while(head != NULL){
		if(equal(head->contact, contact) == 0){
			found = head;
		}
	}
	return found;
}


