/*
 * Header file for Project 3 imported Queue data structure source
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

typedef struct Node {
    int socket_desc;
    struct Node *next;
} Node;

typedef struct Queue{
    Node *front;
    Node *back;
} Queue;

/* Queue Function Declarations */
Node *newNode(int socket_desc);
Queue *newQueue();
void enqueue(Queue *q, int socket_desc);
int dequeue(Queue *q);

