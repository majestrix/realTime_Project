#include "local.h"
void printShmem(memory *mp);
key_t genKey ( char key_id );
int initsem(key_t key, int nsems);

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

	key = genKey('A');
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

/* generate key for shmem/sem*/
key_t genKey ( char key_id )
{
	key_t key;
	if ((key = ftok(".", 'J')) == -1) {
		perror("ftok");
		return -1;
	}
	return key;
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

/* 
 * Took from Beej's Guide to Unix IPCs
 */
int initsem(key_t key, int nsems)  /* key from ftok() */
{
	int i;
	union semun arg;
	struct semid_ds buf;
	struct sembuf sb;
	int semid;

	semid = semget(key, nsems, IPC_CREAT | IPC_EXCL | 0666);

	if (semid >= 0) { /* we got it first */
		sb.sem_op = 1; sb.sem_flg = 0;
		arg.val = 1;

		for(sb.sem_num = 0; sb.sem_num < nsems; sb.sem_num++) { 
			/* do a semop() to "free" the semaphores. */
			/* this sets the sem_otime field, as needed below. */
			if (semop(semid, &sb, 1) == -1) {
				int e = errno;
				semctl(semid, 0, IPC_RMID); /* clean up */
				errno = e;
				return -1; /* error, check errno */
			}
		}

	} else if (errno == EEXIST) { /* someone else got it first */
		int ready = 0;

		semid = semget(key, nsems, 0); /* get the id */
		if (semid < 0) return semid; /* error, check errno */

		/* wait for other process to initialize the semaphore: */
		arg.buf = &buf;
		for(i = 0; i < MAX_RETRIES && !ready; i++) {
			semctl(semid, nsems-1, IPC_STAT, arg);
			if (arg.buf->sem_otime != 0) {
				ready = 1;
			} else {
				sleep(1);
			}
		}
		if (!ready) {
			errno = ETIME;
			return -1;
		}
	} else {
		return semid; /* error, check errno */
	}

	return semid;
}

