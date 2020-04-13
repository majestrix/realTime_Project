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
	return q->count == MAX_Q;
}

int queueSize(queue* q) {
	return q->count;
}  

int insert(queue* q,int data) {

	if(!isFull(q)) {
		int rear = q->rear;	
		if(rear == MAX_Q-1) {
			q->rear = -1;            
		}       
		q->arr[++rear] = data;
		q->rear = rear;
		q->count++;
		return 1;
	}else{
		return 0;
	}
}

int removeData(queue* q) {
	int front = q->front;
	int data = q->arr[front++];
	q->front = front;

	if(front == MAX_Q) {
		q->front = 0;
	}
	q->count++;
	return data;  
}

void initQueue(queue* q){
	q->front = 0;
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


