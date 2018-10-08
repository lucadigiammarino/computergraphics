#pragma once
#pragma comment(lib,"linkedList")
#include "doublyLinkedList.h"

struct Line {
	struct Line *next;
	float m_position[4];
	float *m_color;
};

extern struct Line *head;

void print_list();
Line *create_line(float *m_position);
void insert_first(Line *pLine);
//bool remove_line(Node *pPlanet);
Line* delete_first();
