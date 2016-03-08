/*
	Michal Zagorski
	Systemy Operacyjne 2015/2016
	2 marca 2016
*/

#include "mz_list.h"


void add_next (Node_t* last, Node_t* inserted);
void add_prev (Node_t* head, Node_t* inserted);
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



Node_t* delete_node(Node_t* head){
	Node_t* new_head = NULL;
	if(head->prev != NULL){
		head->prev->next = head->next;
		new_head = head->prev;
	}
	remove_contact_data(head->contact);
	if(head->next != NULL){
		head->next->prev = head->prev;
		if(new_head == NULL){
			new_head = head->next;
		}
	}

	free(head);
	return new_head;
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
		head = delete_node(head);
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





Node_t* search_contact(Node_t* head, Contact_t contact){
	Node_t* found = NULL;
	while(head != NULL){
		if(equal(head->contact, contact) == 0){
			found = head;
		}
	}
	return found;
}


Node_t* swap_in_list(Node_t* left, Node_t* right){
	/* List interpreted as head on the left side */
	Node_t* head = NULL;
	if(left == NULL){
		head = right;
	}else if(right == NULL){
		head = left;
	}else{
		right->prev = left->prev;
		if(right->prev != NULL){
			right->prev->next = right;
		}
		left->next = right->next;
		if(left->next != NULL){
			left->next->prev = left;
		}
		left->prev = right;
		right->next = left;
		head = left;
	}
	return head;
}

Node_t* sort_list(Node_t* head){
	/*
	Bubble sort on lists
	head -> the most left node
	Mowing right
	*/
	Node_t* end = NULL;
	Node_t* left = head;
	Node_t* right = NULL;
	if(head != NULL){
		right = head->next;
	}
	if(right != NULL){
		while(head != end){
			if(compare_contacts(&(head->contact), &(head->next->contact)) > 0){
				head = swap_in_list(head, head->next);
				left = head;
				right = head->next;
			}
			while(right != end){
				if(compare_contacts(&(left->contact), &(right->contact)) > 0){
					right = swap_in_list(left, right)->next;
				}
				left = right;
				right = left->next;
			}
			end = left;
		}
	}
	return head;
}
