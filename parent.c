#include "local.h"
#include "queue.h"
#include "ipc_functions.h"

int main ( int argc, char *argv[] )
{
	int    shmid,semid,i;
	int    status = 0;
	key_t  key;
	pid_t  doctors[NUMBER_OF_DOCTORS];
	pid_t  fork_returnVal;
	memory *mp;


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
                                                /* two semaphores */
	key   = genKey('A');                    /* atomic read&write */
	semid = initsem(key, 2);                /* for both doctor&patient */

	/*-----------------------------------------------------------------------------
	 *  INIT FORKS & Q
	 *-----------------------------------------------------------------------------*/

	initQueue(&(mp->patientQueue));         /* Queue implementation */

	for(i = 0; i < 2; i++){ /* Fork doctors */

		if( (doctors[i] = fork() ) == -1){
			perror("parent -- fork doctor");
			return EXIT_FAILURE;
		}else if(doctors[i] == 0){
			char argtxt[15]={0};
			sprintf(argtxt,"%d %d",shmid,semid);
			execlp("./doctor","./doctor",argtxt,(char*)NULL);
			perror("parent -- exec doctor");
			return EXIT_FAILURE;
		}
	}

	/* THIS IS TEMPRORARY! */
	for(i = 0; i < 5; i++){                 /* Fork Patients */
		if((fork_returnVal = fork()) == -1){
			perror("parent -- fork patient");
			return EXIT_FAILURE;
	        }else if(fork_returnVal == 0){
			char argtxt[15]={0};
			sprintf(argtxt,"%d %d",shmid,semid);
			execlp("./patient","./patient",argtxt,(char*)NULL);
			perror("parent -- exec patient");
			return EXIT_FAILURE;
		
		}
	
	}


	/*-----------------------------------------------------------------------------
	 *  PARENT CODE
	 *-----------------------------------------------------------------------------*/
/* 
 * 1. Fork using threads/with monitoring
 * 2. Monitoring kollo ;-;
 * 3. Parent exit (threshold/counters)
 * 4. Sleep times
 */
	while( wait(&status) > 0);
	
	printf("Contents of sharedmemory:\n");
	printShmem(mp);

	/*-----------------------------------------------------------------------------
	 *  Delete IPCS
	 *-----------------------------------------------------------------------------*/

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


