#ifndef  LOCAL_INC
#define  LOCAL_INC
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <wait.h>
#include <sys/sem.h>

#define NUMBER_OF_DOCTORS         10
#define FORK_NEW_PATIENT          5
#define MAX_PATIENTS              100
#define MAX_Q NUMBER_OF_DOCTORS * 2

union semun {
	int val;               /* used for SETVAL only */
	struct semid_ds *buf;  /* used for IPC_STAT and IPC_SET */
	ushort *array;         /* used for GETALL and SETALL */
};


typedef struct queueStruct 
{ 
	int front, rear,count;
	int arr[MAX_Q]; 
}queue; 


typedef struct memstruct{
	int doctorCount;                        /* to track doctors array */
	int doctors[NUMBER_OF_DOCTORS];
	queue patientQueue;                     /* array queue implementation */
} memory;                                       /* since we don't know the number. */

typedef struct childstruct{
	pid_t pid;
	int status;
} child;
#endif 
