#include "local.h"
#include "ipc_functions.h"

void printShmem(memory *mp);

int main ( int argc, char *argv[] )
{
	int             shmid,semid,i;
	int   	        status = 0;
	key_t           key;
	child           doctors[NUMBER_OF_DOCTORS];
	memory          *mp;

	/*-----------------------------------------------------------------------------
	 *  SHARED MEMORY INIT
	 *-----------------------------------------------------------------------------*/
	if( (shmid = shmget(0x1234, sizeof(memory), 0666|IPC_CREAT)) == -1)
	{
		perror("parent -- shmgt");
		return EXIT_FAILURE;
	}

	if( ( mp = shmat(shmid, NULL, 0) ) == (void*) -1)
	{
		perror("parent -- shmat");
		return EXIT_FAILURE;
	}

	/*-----------------------------------------------------------------------------
	 *  SEMAPHORE INIT
	 *-----------------------------------------------------------------------------*/

	key   = genKey('A');
	semid = initsem(key, NUMBER_OF_DOCTORS);

	/*-----------------------------------------------------------------------------
	 *  INIT FORKS
	 *-----------------------------------------------------------------------------*/
	for(i = 0; i < NUMBER_OF_DOCTORS; i++){

		if( (doctors[i].pid = fork() ) == -1){
			perror("parent -- fork");
			return EXIT_FAILURE;
		}else if(doctors[i].pid == 0){
			char shmtxt[5],semtxt[5];
			sprintf(shmtxt,"%d",shmid);
			sprintf(semtxt,"%d",semid);
			execlp("./doctor","./doctor",shmtxt,semtxt,(char*)NULL);
			perror("parent -- exec");
			return EXIT_FAILURE;
		}
	}

	/*-----------------------------------------------------------------------------
	 *  PARENT CODE
	 *-----------------------------------------------------------------------------*/
	while( wait(&status) > 0);
	
	printf("Contents of sharedmemory:\n");
	printShmem(mp);

	if(shmctl(shmid, IPC_RMID, (struct shmid_ds *) 0) == -1 ){
		perror("parent -- remove shm");
		return EXIT_FAILURE;
	}
	if(semctl(semid,0, IPC_RMID)  == -1 ){
		perror("parent -- remove sem");
		return EXIT_FAILURE;
	}
	printf ( "Pid: %d \n", getpid() );
	return EXIT_SUCCESS;
}


/* to print shared memor contents, effectively PIDS of doctors/patients*/
void printShmem(memory *mp){
	for(int i = 0; i < NUMBER_OF_DOCTORS; i++)
		printf("%d | ", mp->doctors[i]);
	printf ( "\n" );
	for(int i = 0; i < MAX_PATIENTS; i++)
		printf("%d | ", mp->patients[i]);
	printf ( "\n" );
}

