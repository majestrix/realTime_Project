#include "local.h"
void lock(int semid, struct sembuf *sb, int semnum);
void unlock(int semid, struct sembuf *sb, int semnum);

int main ( int argc, char *argv[] )
{
	int    shmid,semid,len;
	char* str;
	struct sembuf   sb;
	memory *mp;
	
	len = strlen(argv[1]) - strlen(argv[2]);
	str = malloc(sizeof(char) * len);
	memcpy(str,argv[1],len);
	shmid = atoi(str);
	semid = atoi(argv[2]);
	
	if(argc != 3)
	{
		printf("Usage ./doctor [memory id] [semaphore id]\n");
		return EXIT_FAILURE;
	}
	if( ( mp = shmat(shmid, NULL, 0) ) == (void*) -1)
	{
		perror("doctor -- shmat");
		return EXIT_FAILURE;
	}

	sb.sem_flg = SEM_UNDO;
	lock(semid,&sb,getpid()%10);
	mp->doctors[mp->front++] = getpid();
	unlock(semid,&sb,getpid()%10);

	printf ( "Child Pid: %d\n", getpid() );
	return EXIT_SUCCESS;
}

/* lock() - locks the semaphore, blocking other processes */
void lock(int semid, struct sembuf *sb, int semnum) {
	/* lock the semaphore */
	sb->sem_num = semnum;
	sb->sem_op = -1; /* allocate resource */
	if (semop(semid, sb, 1) == -1) {
		perror("semop");
		exit(1);
	}
}

/* unlock() - unlocks the semaphore, allowing access by other processes */
void unlock(int semid, struct sembuf *sb, int semnum) {
	/* unlock the semaphore */
	sb->sem_num = semnum;
	sb->sem_op = 1; /* free resource */
	if (semop(semid, sb, 1) == -1) {
		perror("semop");
		exit(1);
	}
}
