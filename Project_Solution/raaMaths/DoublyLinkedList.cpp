#include <stdio.h>
#include <conio.h>
#include <string.h>


struct Node {
	Node *m_pNext;
	Node *m_pLast;
	int m_pvalue;
};

Node *g_pHead = 0;
Node *g_pTail = 0;

Node *createNewElement(int l_ix) {
	Node *pElement = new Node; //using new operator to create new Node
	pElement->m_pvalue = l_ix;
	pElement->m_pLast = 0; //pointers initialised to 0
	pElement->m_pNext = 0;
	return pElement;
}

void pushToHead(int l_ix) {
	Node *pElement = createNewElement(l_ix);
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

void pushToTail(int l_ix) {
	struct Node *pElement = createNewElement(l_ix);
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


Node *search(int l_ix) {
	unsigned int uiCont = 0;
	for (Node *pElement = g_pHead; pElement; pElement = pElement->m_pNext) {
		if (l_ix == pElement->m_pvalue) {
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
	bool flag = false;
	if (pElement != 0) { //check to make sure is a valid element
						 //if (pElement->m_pNext != 0 & pElement->m_pLast != 0) { //check if the element is a member of the list
		if (g_pHead == g_pTail) { //checks if there is only one item in the list
			g_pHead = g_pTail = 0; //set both head and tail to 0
			pElement = g_pHead; //set element to be deleted
			flag = true;
		}
		else if (pElement == g_pHead) { //check to see if element address matches head
			flag = destroy(popHead());
		}
		else if (pElement == g_pTail) { //check to see if element address matches tail
			flag = destroy(popTail());
		}
		else { //if element is not head or tail and is just a component of the list
			Node *pPrevious, *pUpcoming = 0;
			pPrevious = pElement->m_pLast; //storing address to temporary variables
			pUpcoming = pElement->m_pNext;

			pPrevious->m_pNext = pUpcoming; //setting pointers of temporary variables
			pUpcoming->m_pLast = pPrevious;

			delete pElement;
			flag = true;
		}
		//}
	}
	return flag;
}



void printList() {
	unsigned int uiCont = 0;
	for (Node *pElement = g_pHead; pElement; pElement = pElement->m_pNext) {
		printf("Element: %d\n", uiCont++);
		printf("\tValue:%d\n", pElement->m_pvalue);
	}
}

/**
int main() {


	//pushToTail(40);
	//pushToHead(10);
	//pushToHead(20);
	pushToHead(30);

	//insertAfter(createNewElement(70), search(30));
	//insertBefore(createNewElement(100), search(30));
	//remove(search(100));
	//destroy(popHead());


	pushToTail(50);
	pushToTail(60);
	pushToTail(70);
	insertAfter(createNewElement(80), search(70));
	insertBefore(createNewElement(40), search(50));

	remove(search(40));
	remove(search(50));
	remove(search(60));
	remove(search(70));
	remove(search(80));

	/*
	search(20);

	printf("Value Popped (Head): %d\n", popHead()->m_pvalue); //30
	printf("Value Popped (Tail): %d\n", popTail()->m_pvalue); //50

	//destroying the var
	Node *pElement = 0;
	int temp = 0;
	pElement = popHead();
	temp = pElement->m_pvalue;
	if (destroy(pElement)) {
	printf("The value destroyed is: %d\n", temp); //20
	}
	else {
	printf("No values destroyed");
	}

	
	printList();

	getchar();
	
}
**/