#include "local.h"

int main ( int argc, char *argv[] )
{
	int    shmid,semid,len;
	char* str;
	struct sembuf   sb;
	memory *mp;

	/* For some reason arguments are concatenated in argv1*/
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
	
	if( shmdt(mp) == -1)
	{
		perror("doctor -- shmem detach");
		return EXIT_FAILURE;
	}
	
	printf ( "Child Pid: %d\n", getpid() );
	return EXIT_SUCCESS;
}

