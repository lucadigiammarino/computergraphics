#pragma once
#pragma comment(lib,"linkedList")

struct Data {
	Data *m_pNextData;
	float m_data_position[4];
};


struct Node {
	Node *m_pNext;
	Node *m_pLast;
	float m_position[4];
	float m_mass;
	float *m_pcolor;
	float m_velocity[4];
	Data *registerPosition;
	float m_camera;
};



extern Node *g_pHead;
extern Node *g_pTail;

Node *createNewElement(float *m_position, float m_mass, float *m_pcolor, Data *registerPosition, int m_camera);
Node *popHead();
Node *popTail();
Node *searchColor(float *m_pcolor);
Node *searchSize(float m_mass);

void pushToHead(Node *pElement);
void pushToTail(Node *pElement);
void insertBefore(Node *pElement, Node *pTarget);
void insertAfter(Node *pElement, Node *pTarget);
void printList();

bool destroy(Node *pElement);
bool remove(Node *pElement);



/**SINGLE LINKED LIST**/

Data *createNewData(float *m_position);
void pushToHeadData(Data *newNode, Node *node);