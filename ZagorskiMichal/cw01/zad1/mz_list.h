/*
	Michal Zagorski
	Systemy Operacyjne 2015/2016
	2 marca 2016
*/

#ifndef __MZ_LIST_H
#define  __MZ_LIST_H

#include "mz_contact.h"
// #include <stdlib.h>
// #include <errno.h>
// #include <string.h>
// #include <stdio.h>

struct Node {
	struct Contact contact;
	struct Node* next;
	struct Node* prev;
};
typedef struct Node Node_t;


Node_t* create_list (void);
Node_t* delete_list (Node_t* head);
Node_t* push_back (Node_t* head, Node_t* inserted);
//void add_next (Node_t* last, Node_t* inserted);
//void add_prev (Node_t* head, Node_t* inserted);
Node_t* push_front(Node_t* node, Node_t* inserted);
Node_t* sort_list(Node_t* head);
Node_t* search_contact(Node_t* head, Contact_t contact);
//Node_t* delete_node(Node_t* node);



#endif
