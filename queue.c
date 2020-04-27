#include "local.h"
#include "queue.h"
/* 
 * Pretty self explanatory...
 */
int peek(queue* q) {
	return q->arr[q->front];
}

int isEmpty(queue* q) {
	return q->count == 0;
}

int isFull(queue* q) {
	int front = q->front;
	int rear = q->rear;
	if ((front == 0 && rear == MAX_Q-1) || 
			(rear == (front-1)%(MAX_Q-1))) 
	{ 
		return 1; 
	} 
	return 0;
}

int queueSize(queue* q) {
	return q->count;
}  

int insert(queue* q,int data) {
	int front = q->front;
	int rear = q->rear;
	if ((front == 0 && rear == MAX_Q-1) || 
			(rear == (front-1)%(MAX_Q-1))) 
	{ 
		printf("F%d,R%d\n",front,rear);
		printf("\nQueue is Full\n"); 
		return -1; 
	} 

	else if (front == -1) /* Insert First Element */
	{ 
		front = rear = 0; 
		q->arr[rear] = data; 
	} 

	else if (rear == MAX_Q-1 && front != 0) 
	{ 
		rear = 0; 
		q->arr[rear] = data; 
	} 

	else
	{ 
		rear++; 
		q->arr[rear] = data; 
	} 
	q->count++;
	q->front = front;
	q->rear = rear;
	return 1;
}

int dequeue(queue* q) {
	int front = q->front;
	int rear = q->rear;
	if (front == -1) 
	{ 
		return -1; 
	} 

	int data;
	while( (data = q->arr[front] ) == -1) front++;
	q->arr[front] = -1; 
	if (front == rear) 
	{ 
		front = -1; 
		rear = -1; 
	} 
	else if (front == MAX_Q-1) 
		front = 0; 
	else
		front++; 

	q->count--;
	q->front = front;
	q->rear = rear;
	return data;
}

int removeData(queue* q,int data){
	for (int i = q->front ;i <= q->rear ; i++)
		if(q->arr[i] == data)
		{
			q->arr[i] = -1;
			q->count--;
			return 1;
		}
	return 0;
}

void initQueue(queue* q){
	q->front = -1;
	q->rear = -1;
	q->count = 0;
	return;
}

void printQueue(queue* q){
	if(isEmpty(q))
		printf ( "Queue empty.\n" );
	else{
		for (int i = q->front ;i <= q->rear ; i++)
			printf("%d | ",q->arr[i]);
	}
	return;
}

