#include<stdio.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/types.h>
#include<string.h>
#include<errno.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<wait.h>
#include <sys/sem.h>

#define NUMBER_OF_DOCTORS 10
#define FORK_NEW_PATIENT  5
#define MAX_PATIENTS      10
#define COVID_19_FEVER    0
#define COVID_19_COUGH    0
#define COVID_19_BREATH   0
#define BLOOD_HYPER       0
#define HEART_RESP        0
#define PATIENT_AGE       0

#define MAX_RETRIES 10

union semun {
    int val;               /* used for SETVAL only */
    struct semid_ds *buf;  /* used for IPC_STAT and IPC_SET */
    ushort *array;         /* used for GETALL and SETALL */
};

typedef struct memstruct{
	int front;
	int doctors[NUMBER_OF_DOCTORS];
	int patients[MAX_PATIENTS];
} memory;

typedef struct childstruct{
	pid_t pid;
	int status;
} child;
