#include "serverLib.h"

Node *newNode(int socket_desc){
	Node *n = (Node *)malloc(sizeof(Node));
	n->socket_desc = socket_desc;
	n->next = NULL;
	return n;
}



Queue *newQueue(){
	Queue *q = (Queue*)malloc(sizeof(Queue));
	q->front = NULL;
	q->back = NULL;
	return q;
}


void enqueue(Queue *q, int socket_desc){
	Node *temp = newNode(socket_desc);
	
	if(q->back == NULL){
		q->front = temp;
		q->back = temp;
	} else {
		q->back->next = temp;
		q->back = temp;
	}
}

int dequeue(Queue *q){
	
	if (q->front ==NULL){
		return -1;
	}
	
	Node *temp = q->front;
	q->front = q->front->next;
	
	if (q->front == NULL){
		q->back == NULL;
	}
	
	return temp->socket_desc;
}

