#pragma once
#pragma comment(lib,"doublyLinkedList")


struct Node;

Node *g_pHead = 0;
Node *g_pTail = 0;

Node *createNewElement(int l_ix);
Node *popHead();
Node *popTail();
Node *search(int l_ix);

void pushToHead(int l_ix);
void pushToTail(int l_ix);
void insertBefore(Node *pElement, Node *pTarget);
void insertAfter(Node *pElement, Node *pTarget);
void printList();

bool destroy(Node *pElement);
bool remove(Node *pElement);