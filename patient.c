#include "local.h"
#include "ipc_functions.h"
#include "queue.h"
#include <unistd.h>
#include <math.h>

int generateNumber (int low, int high);
int isSevere(int num);
int healthCondition(int s, int pid);
int healthSeverity(int fever,int cough,int breath ,int hyper,int heart,int cancer);
int ageSeverity(int age);

int main ( int argc, char *argv[] )
{
	int           shmid,semid,len;
	int           severity = 0, health=1;
	char*         str;
	memory        *mp;
	struct sembuf sb;

	int fever  = isSevere( generateNumber(0,8) );
	int cough  = isSevere( generateNumber(0,8) );
	int breath = isSevere( generateNumber(0,8) );
	int hyper  = isSevere( generateNumber(0,8) );
	int heart  = isSevere( generateNumber(0,8) );
	int cancer = isSevere( generateNumber(0,8) );
	int age    = generateNumber(10,110);


	/* For some reason arguments are concatenated in argv1*/
	len = strlen(argv[1]) - strlen(argv[2]);
	str = malloc(sizeof(char) * len);
	memcpy(str,argv[1],len);
	shmid = atoi(str);
	semid = atoi(argv[2]);

	if(argc != 3)
	{
		printf("Usage ./patient [memory id] [semaphore id]\n");
		return EXIT_FAILURE;
	}

	if( ( mp = shmat(shmid, NULL, 0) ) == (void*) -1)
	{
		perror("patient -- shmat");
		return EXIT_FAILURE;
	}
	

	/* Enqueue patient */
	printf ( "Patient writing\n" );
	sb.sem_flg = SEM_UNDO;
	lock(semid,&sb,1);
	insert(&(mp->patientQueue) ,getpid() ); 
	unlock(semid,&sb,1);

	/* Initial severity */
	severity  = healthSeverity(fever,cough,breath,hyper,heart,cancer);
	severity += ageSeverity(age);

	/* Monitor health */
//	while(1){
//	       /* Wait for doctor signal and break */
//	       /* or Increment severity every 1sec */
//		health = healthCondition(severity++,getpid());
//		if(health == 0) break;
//		sleep(1);
//	}
	
       /* Doctor-Patient communication here */
	
	if( shmdt(mp) == -1)
	{
		perror("doctor -- shmem detach");
		return EXIT_FAILURE;
	}

	if(health == 0)
		return 10;
	return EXIT_SUCCESS;
}


int generateNumber (int low, int high)
{
	return rand() % (high - low + 1) + low;
}
int isSevere(int num)
{
	return (num > 4) ? 1 : 0;
}

int ageSeverity(int age){
	/* map age severity to exponential function 0-10 */
	return floor( (age*age)/1100 );
}

int healthSeverity(int fever,int cough,int breath ,int hyper,int heart,int cancer)
{
	int severity=0;
	severity = fever  == 1 ? severity+1 :  severity;
	severity = cough  == 1 ? severity+1 :  severity;
	severity = breath == 1 ? severity+1 :  severity;
	severity = hyper  == 1 ? severity+1 :  severity;
	severity = heart  == 1 ? severity+2 :  severity;
	severity = cancer == 1 ? severity+3 :  severity;
	return severity;
}

int healthCondition(int s, int pid){
	if (s > 15 ){
		return 0;
	}
	return 1;
}
