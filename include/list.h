
#ifndef LIST_H
#define LIST_H

#include <stdlib.h>

typedef struct object {
	void *data;
	struct object *next;
} E;

E *list__new_node(void *data){
	E *node = malloc(sizeof(E));
	node->data = data;
	node->next = NULL;
	return node;
}//list__new_node

E *list_create(){
	E *e = list__new_node(NULL);
	return e;
}//list_create

int list_size(E *list){
	int size = 0;

	//Advance past first holder
	list = list->next;
	while (list && list->data){
		list = list->next;
		size++;
	}
	return size;
}//list_size

void *list_get(E *list, int index){
	void* item = NULL;

	//Advance past first holder
	list = list->next;
	int scan_i = 0;
	while (list){
		if (scan_i == index){
			item = list->data;
			break;
		}
		list = list->next;
		scan_i++;
	}
	return item;
}//list_get

void list_append(E *list, void *item){
	//Create list element
	E *node = list__new_node(item);

	//Scan to the end of the list
	while (list->next)
		list = list->next;

	list->next = node;
}//list_append

void list_insert(E *list, void *item, int index){
	//Create list element
	E *node = list__new_node(item);
	
	//If list is empty
	if (!list->next){
		list->next = node;
		return;
	}

	//Scan until end-of-list or through index elements
	int scan_i = 0;
	while (scan_i < index && list->next){
		list = list->next;
		scan_i++;
	}

	if (list->next)
		node->next = list->next;

	list->next = node;
}//list_insert

void list_remove(E *list, int index){
	//If list is empty
	if (!list->next)
		return;
	
	int scan_i = 0;
	while (scan_i < index){
		list = list->next;
		scan_i++;
		if (!list->next) return;
	}

	E *afterPart = list->next->next;
	list->next = afterPart;
}//list_remove    DOES NOT FREE SPACE


#endif
