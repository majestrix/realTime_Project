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

typedef struct memstruct{
	int a1[10];
	int a2[10];
} memory;

int main ( int argc, char *argv[] )
{
	int shmid,i;
	int status = 0;
	pid_t pid;
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
		mp->a1[i] = i;
		mp->a2[i] = 10-i;
	}	

	printf ( "Before forking doctor\n" );
	for(int i = 0; i < 10; i++)
		printf("%d | ", mp->a1[i]);
	printf ( "\n" );
	for(int i = 0; i < 10; i++)
		printf("%d | ", mp->a2[i]);
	printf ( "\n" );

	if( (pid = fork() ) == -1){
		perror("parent -- fork");
		return 100;
	}else if(pid == 0){
		execlp("./doctor","./doctor",(char*) NULL); /* doctor refills shmem arrays with 100 +- i */
		perror("parent -- exec");
		return 2;
	}
	else{
		while(wait(&status) > 0);

		printf ( "After forking doctor: %d\n", getpid() );
		for(int i = 0; i < 10; i++)
			printf("%d | ", mp->a1[i]);
		printf ( "\n" );
		for(int i = 0; i < 10; i++)
			printf("%d | ", mp->a2[i]);	
		printf ( "\n" );

		if(shmctl(shmid, IPC_RMID, (struct shmid_ds *) 0) == -1 ){
			perror("parent -- remove shm");
			return 97;
		}
	}
	return EXIT_SUCCESS;
}
