#include <stdio.h>
#include <conio.h>
#include <string.h>
#include "doublyLinkedList.h"

Node *g_pHead = 0;
Node *g_pTail = 0;

Node *createNewElement(float *m_position, float m_mass, float *m_pcolor, Data *registerPosition, int m_camera) {
	Node *pElement = new Node; //using new operator to create new Node
	pElement->m_position[0] = m_position[0]; //storing x, y, z
	pElement->m_position[1] = m_position[1];
	pElement->m_position[2] = m_position[2];
	pElement->m_position[3] = 1.0f;
	pElement->m_mass = m_mass;
	pElement->m_pcolor = m_pcolor;
	pElement->m_pLast = 0; //pointers initialised to 0
	pElement->m_pNext = 0;
	//store position history to draw lines
	pElement->registerPosition = registerPosition;
	pElement->m_camera = m_camera;
	return pElement;
}

void pushToHead(Node *pElement) {
	if (pElement->m_pLast == 0 && pElement->m_pNext == 0) { //checks if the new element is not linked with the list
		if (g_pHead == 0 && g_pTail == 0 && g_pHead == g_pTail) { //checks if there are any items in the list //the check here should be g_pHead == g_pTail == 0
			g_pHead = g_pTail = pElement;
			return;
		}
		g_pHead->m_pLast = pElement;
		pElement->m_pNext = g_pHead;
		g_pHead = pElement;
	}
}

void pushToTail(Node *pElement) {
	if (pElement->m_pLast == 0 && pElement->m_pNext == 0) { //checks if the new element is not linked with the list
		if (g_pHead == 0 && g_pTail == 0 && g_pHead == g_pTail) { //checks if there are any items in the list
			g_pHead = g_pTail = pElement;
			return;
		}
		g_pTail->m_pNext = pElement;
		pElement->m_pLast = g_pTail;
		g_pTail = pElement;
	}
}

Node *popHead() {
	Node *pElement = 0;
	if (g_pHead == 0 && g_pTail == 0 && g_pHead == g_pTail) { //check if there are any element on the list
		return 0;
	}
	else {
		if (g_pHead == g_pTail) { //checks if there is only one item in the list
			pElement = g_pHead; // set the element to be returned
			g_pHead = g_pTail = 0; //set both head and tail to 0
		}
		else {
			Node *temp = g_pHead;
			g_pHead = temp->m_pNext;
			g_pHead->m_pLast = 0;
			pElement = temp;
		}
	}
	return pElement;
}

Node *popTail() {
	Node *pElement = 0;
	if (g_pHead == 0 && g_pTail == 0 && g_pHead == g_pTail) { //check if there are any element on the list
		return 0;
	}
	else {
		if (g_pHead == g_pTail) { //checks if there is only one item in the list
			pElement = g_pTail; // set the element to be returned
			g_pHead = g_pTail = 0;  //set both head and tail to 0
		}
		else {
			Node *temp = g_pTail;
			g_pTail = temp->m_pLast;
			g_pTail->m_pNext = 0;
			pElement = temp;
		}
	}
	return pElement;
}



Node *searchColor(float *m_pcolor) {
	unsigned int uiCont = 0;
	for (Node *pElement = g_pHead; pElement; pElement = pElement->m_pNext) {
		if (m_pcolor == pElement->m_pcolor) {
			return pElement;
		}
		uiCont++;
	}
}



Node *searchSize(float m_radius) {
	unsigned int uiCont = 0;
	for (Node *pElement = g_pHead; pElement; pElement = pElement->m_pNext) {
		if (m_radius == pElement->m_mass) {
			return pElement;
		}
		uiCont++;
	}
}

void insertBefore(Node *pElement, Node *pTarget) {
	if (pElement->m_pLast == 0 && pElement->m_pNext == 0) { //checks if the new element is not linked with the list
		if (pTarget == g_pHead) { //check if target is the current head
			g_pHead->m_pLast = pElement; //use the same strategy for push head
			pElement->m_pNext = g_pHead;
			g_pHead = pElement;
		}
		else {
			Node *pPrevious = 0; //initialising temp pointer previous
			pPrevious = pTarget->m_pLast; //getting the position of the temporary prvious pointer

			pPrevious->m_pNext = pElement; //previous next pointer pointing to the new inserted element
			pElement->m_pLast = pPrevious; //new inserted element last pointer pointing to previous element

			pElement->m_pNext = pTarget; //new element next pointer pointing to target
			pTarget->m_pLast = pElement; //target last pointer pointing to new inserted element
		}
	}
}

void insertAfter(Node *pElement, Node *pTarget) {
	if (pElement->m_pLast == 0 && pElement->m_pNext == 0) { //checks if the new element is not linked with the list
		if (pTarget == g_pTail) { //check if target is the current tail
			g_pTail->m_pNext = pElement; //use the same strategy for push tail
			pElement->m_pLast = g_pTail;
			g_pTail = pElement;
		}
		else {
			Node *pUpcoming = 0; //initialising temp pointer upcoming
			pUpcoming = pTarget->m_pNext; //getting the position of the upcoming pointer

			pElement->m_pNext = pUpcoming; //new element next pointer pointing to the upcoming element
			pUpcoming->m_pLast = pElement; //upcoming element last pointer pointing to the new element

			pElement->m_pLast = pTarget; //new element last pointer pointing to the target element
			pTarget->m_pNext = pElement; //target next pointer pointing to the new element
		}
	}
}


bool destroy(Node *pElement) {
	if (pElement != 0) { //check to make sure is a valid element
		if (pElement->m_pNext == 0 && pElement->m_pLast == 0) { //check to see if element is not a member of the list
			if (pElement != g_pHead && pElement != g_pTail) { //check to see if element address does not match head or tail
				delete pElement;
				return true;
			}
		}
	}
}

bool remove(Node *pElement) {
	bool isElementRemoved = false;
	if (pElement != 0) { //check to make sure is a valid element
						 //if (pElement->m_pNext != 0 & pElement->m_pLast != 0) { //check if the element is a member of the list
		if (g_pHead == g_pTail) { //checks if there is only one item in the list
			g_pHead = g_pTail = 0; //set both head and tail to 0
			pElement = g_pHead; //set element to be deleted
			isElementRemoved = true;
		}
		else if (pElement == g_pHead) { //check to see if element address matches head
			isElementRemoved = destroy(popHead());
		}
		else if (pElement == g_pTail) { //check to see if element address matches tail
			isElementRemoved = destroy(popTail());
		}
		else { //if element is not head or tail and is just a component of the list
			Node *pPrevious, *pUpcoming = 0;
			pPrevious = pElement->m_pLast; //storing address to temporary variables
			pUpcoming = pElement->m_pNext;

			pPrevious->m_pNext = pUpcoming; //setting pointers of temporary variables
			pUpcoming->m_pLast = pPrevious;

			delete pElement;
			isElementRemoved = true;
		}
	}
	return isElementRemoved;
}



void printList() {
	unsigned int uiCont = 0;
	for (Node *pElement = g_pHead; pElement; pElement = pElement->m_pNext) {
		printf("Element: %d\n", uiCont++);
		printf("\t %f | | %f | | %f", pElement->m_position[0], pElement->m_position[1], pElement->m_position[2]);
		printf("\tColor:%lf\n", pElement->m_pcolor); //to revise this value, to print the value of the color
		printf("\tRadius:%f\n", pElement->m_mass);
	}
}

/************SINGLE LINKED LIST*************/


void pushToHeadData(Data *newNode, Node *node) {
	newNode->m_pNextData = node->registerPosition;
	node->registerPosition = newNode;
}


Data *createNewData(float *m_position) {
	Data *data = new Data;
	data->m_data_position[0] = m_position[0];
	data->m_data_position[1] = m_position[1];
	data->m_data_position[2] = m_position[2];
	data->m_pNextData = 0;
	return data;
}
