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

#define NUMBER_OF_DOCTORS 10
#define FORK_NEW_PATIENT  5
#define MAX_PATIENTS      10
#define COVID_19_FEVER    0
#define COVID_19_COUGH    0
#define COVID_19_BREATH   0
#define BLOOD_HYPER       0
#define HEART_RESP        0
#define PATIENT_AGE       0


typedef struct memstruct{
	int doctors[NUMBER_OF_DOCTORS];
	int patients[MAX_PATIENTS];
} memory;

