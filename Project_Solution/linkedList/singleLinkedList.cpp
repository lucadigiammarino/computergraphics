#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "singleLinkedList.h"

Line *head = 0;

//display the list
void print_list() {
	unsigned int uiCont = 0;
	for (Line *pElement = head; pElement; pElement = pElement->next) {
		printf("Element: %d\n", uiCont++);
		printf("\t Prev Position: %f | | %f | | %f\n", pElement->m_position[0], pElement->m_position[1], pElement->m_position[2]);
	}
}

Line *create_line(float *m_position) {
	Line *pElement = new Line; //using new operator to create new Node
	pElement->m_position[0] = m_position[0];
	pElement->m_position[1] = m_position[1]; //substitute with copy vec
	pElement->m_position[2] = m_position[2]; 
	return pElement;
}

//insert link at the first location
void insert_first(Line *pLine) {
	//point it to old first node
	pLine->next = head;
	head = pLine;
}

//delete a link with given key
bool remove_line(Node *pPlanet) {
	bool flag = false;

	//start from the first link
	struct Line* current = head;
	struct Line* previous = NULL;

	//if list is empty
	if (head == NULL) {
		flag = false;
	}

	//navigate through list
	while (current->pPlanet != pPlanet) {

		//if it is last node
		if (current->next == NULL) {
			flag = false;
		}
		else {
			//store reference to current link
			previous = current;
			//move to next link
			current = current->next;
			flag = true;
		}
	}

	//found a match, update the link
	if (current == head) {
		//change first to point to next link
		head = head->next;
		flag = true;
	}
	else {
		//bypass the current link
		previous->next = current->next;
		flag = true;
	}
	delete current;
	return flag;
}

//delete first item
struct Line* delete_first() {
	//save reference to first link
	struct Line *tempLink = head;
	//mark next to first link as first 
	head = head->next;
	//return the deleted link
	return tempLink;
}

/**

int main() {

	float v1init[4]; v1init[0] = 1; v1init[1] = 1; v1init[2] = 1;
	float v2init[4]; v2init[0] = 2; v2init[1] = 2; v2init[2] = 2;

	float v3init[4]; v3init[0] = 100; v3init[1] = 100; v3init[2] = 100;
	float v4init[4]; v4init[0] = 200; v4init[1] = 200; v4init[2] = 200;

	node *first = create_node(v1init, v2init);
	node *second = create_node(v3init, v4init);

	insertFirst(first);
	insertFirst(second);

	deleteFirst();

	//print list
	printList();
	getchar();

}

*/