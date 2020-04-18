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

int signal_rcvd = 0;

int main ( int argc, char *argv[] )
{
	int    shmid,semid;
	int    severity = 0, health=1;
	char*  tok;
	memory *mp;
	struct sembuf sb;
	struct msgbuf buf;

	int fever  = isSevere( generateNumber(0,8) );
	int cough  = isSevere( generateNumber(0,8) );
	int breath = isSevere( generateNumber(0,8) );
	int hyper  = isSevere( generateNumber(0,8) );
	int heart  = isSevere( generateNumber(0,8) );
	int cancer = isSevere( generateNumber(0,8) );
	int age    = generateNumber(10,110);


	/* For some reason arguments are concatenated in argv1*/
	tok   = strtok(argv[1]," ");
	shmid = atoi(tok);
	tok   = strtok(NULL," ");
	semid = atoi(tok);

	if(argc != 2)
	{
		printf("Usage ./patient [memory id] [semaphore id]\n");
		return EXIT_FAILURE;
	}
	/* Attach Memory */
	if( ( mp = shmat(shmid, NULL, 0) ) == (void*) -1)
	{
		perror("patient -- shmat");
		return EXIT_FAILURE;
	}
	/* Change signal displacement */
	if ( sigset(SIGUSR1, signal_catcher) == SIG_ERR ) {
		perror("Sigset can not set SIGUSR1");
		return EXIT_FAILURE;
	}

	/* Enqueue patient */
	sb.sem_flg = SEM_UNDO;
	lock(semid,&sb,1);
	insert(&(mp->patientQueue) ,getpid() ); 
	unlock(semid,&sb,1);

	/* Initial severity */
	severity  = healthSeverity(fever,cough,breath,hyper,heart,cancer);
	severity += ageSeverity(age);

	/* Wait for signal */
	while(1){
		while(!signal_rcvd){
			/* Wait for doctor signal and break */
			/* or Increment severity every 1sec */
			health = healthCondition(severity++,getpid());
			if(health == 0) return 10; /* Dead:( */
			sleep(1);
		}
		/* Signal Recieved! */
		/* Communicate with Doctor */
		sprintf(buf.mtext,"%d",severity);
		buf.mtype = 1;
		len = strlen(buf.mtext);
		msqid = /* Find who sent the signal */
		if (msgsnd(msqid, &buf, len+1, 0) == -1) /* +1 for '\0' */
			perror("msgsnd");

		/* Logic Here... */
		/* THIS IS TEMPRORARY! */
		break;
	}

	if( shmdt(mp) == -1)
	{
		perror("doctor -- shmem detach");
		return EXIT_FAILURE;
	}

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

void signal_catcher(int the_sig)
{
	if (the_sig == SIGUSR1);
	signal_rcvd = 1;
}
