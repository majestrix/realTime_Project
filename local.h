#ifndef  LOCAL_INC
#define  LOCAL_INC
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <wait.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/msg.h>

#define BUFF_SIZE         64
#define FORK_NEW_PATIENT  1
#define NUMBER_OF_DOCTORS 10
#define DOCTOR_SLEEP_TIME 15
#define PATIENT_WAIT_TIME 60
#define THRESHOLD         20
#define MAX_Q             200  /* Depends on threshold */

#define PATIENT_DIED      10
#define PATIENT_RECOVERED 20

union semun {
	int val;               /* used for SETVAL only */
	struct semid_ds *buf;  /* used for IPC_STAT and IPC_SET */
	ushort *array;         /* used for GETALL and SETALL */
};

typedef struct queueStruct 
{ 
	int front,rear,count;
	int arr[MAX_Q]; 
} queue; 

typedef struct doctorstruct{
	pid_t pid;
	int   msgqid;
	pid_t myPatient;
} doctor;

typedef struct memstruct{
	int doctorCount;                        /* to track doctors array */
	doctor doctors[NUMBER_OF_DOCTORS];
	queue patientQueue;                     /* array queue implementation */
} memory;                                       /* since we don't know the number. */


struct msgbuf{ 
    long mtype; 
    char mtext[BUFF_SIZE]; 
}; 

#endif 
