#include "local.h"
void printShmem(memory *mp);

int main ( int argc, char *argv[] )
{
	int    shmid,i;
//	int    status = 0;
	child doctors[NUMBER_OF_DOCTORS];
	memory *mp;

	if( (shmid = shmget(0x1234, sizeof(memory), 0666|IPC_CREAT)) == -1)
	{
		perror("parent -- shmgt");
		return 99;
	}

	if( ( mp = shmat(shmid, NULL, 0) ) == (void*) -1)
	{
		perror("parent -- shmat");
		return 98;
	}

	for(i = 0; i < NUMBER_OF_DOCTORS; i++){

		if( (doctors[i].pid = fork() ) == -1){
			perror("parent -- fork");
			return 100;
		}else if(doctors[i].pid == 0){
			char txt[5];
			sprintf(txt,"%d",shmid);
			execlp("./doctor","./doctor",txt,(char*) NULL);
			perror("parent -- exec");
			return 2;
		}
	}

	printf ( "Pid: %d \n", getpid() );
	return EXIT_SUCCESS;
}

void printShmem(memory *mp){
	for(int i = 0; i < NUMBER_OF_DOCTORS; i++)
		printf("%d | ", mp->doctors[i]);
	printf ( "\n" );
	for(int i = 0; i < MAX_PATIENTS; i++)
		printf("%d | ", mp->patients[i]);
	printf ( "\n" );
}
