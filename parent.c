#include "local.h"
void printShmem(memory *mp);

int main ( int argc, char *argv[] )
{
	int    shmid,i;
	int    status = 0;
	pid_t  pid;
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

	for(i=0;i<10;i++){                      /* fill shmem arrays with 0-10 */
		mp->doctors[i] = i;
		mp->patients[i] = 10-i;
	}	

	printf ( "Before forking doctor\n" );
	printShmem(mp);

	if( (pid = fork() ) == -1){
		perror("parent -- fork");
		return 100;
	}else if(pid == 0){
		char txt[5];
		sprintf(txt,"%d",shmid);
		execlp("./doctor","./doctor",txt,(char*) NULL);
		perror("parent -- exec");
		return 2;
	}
	else{
		while(wait(&status) > 0);

		printf ( "After forking doctor: %d\n", getpid() );
		printShmem(mp);

		if(shmctl(shmid, IPC_RMID, (struct shmid_ds *) 0) == -1 ){
			perror("parent -- remove shm");
			return 97;
		}
	}

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
