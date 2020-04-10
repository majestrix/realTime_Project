#include "local.h"

int main ( int argc, char *argv[] )
{
	int    shmid,i;
	memory *mp;
	
	if(argc != 2)
	{
		printf("Usage ./doctor [memory id]\n");
		return 99;
	}
	shmid = atoi(argv[1]);
	if( ( mp = shmat(shmid, NULL, 0) ) == (void*) -1)
	{
		perror("doctor -- shmat");
		return 98;
	}

	printf ( "Child Pid: %d\n", getpid() );
	return EXIT_SUCCESS;
}
