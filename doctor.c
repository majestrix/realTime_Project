#include<stdio.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/types.h>
#include<string.h>
#include<errno.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>

typedef struct memstruct{
	int a1[10];
	int a2[10];
} memory;

int main ( int argc, char *argv[] )
{
	int shmid,i;
	memory *mp;
	if( (shmid = shmget(0x1234,sizeof(memory), 0666|IPC_CREAT) ) == -1)
	{
		perror("doctor -- shmget");
		return 99;
	}
	if( ( mp = shmat(shmid, NULL, 0) ) == (void*) -1)
	{
		perror("doctor -- shmat");
		return 98;
	}
	for(i=0;i<10;i++){
		mp->a1[i] = i+100;
		mp->a2[i] = 100-i;
	}	
	return EXIT_SUCCESS;
}
