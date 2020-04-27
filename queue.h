#ifndef QUEUE_FUNCTIONS
#define QUEUE_FUNCTIONS

int peek(queue* q);
int isEmpty(queue* q);
int isFull(queue* q);
int queueSize(queue* q);
int insert(queue* q,int data);
void initQueue(queue* q);
void printQueue(queue* q);
int removeData(queue* q,int data);
int dequeue(queue* q);

#endif
