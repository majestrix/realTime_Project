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
void signal_catcher(int sig, siginfo_t *si, void *ucontext);
int findDoctorByPid(pid_t pid,memory* mp);

int   signal_rcvd = 0;
pid_t signal_pid;

int main ( int argc, char *argv[] )
{
	int    shmid,semid,msgqid,len;
	int    severity = 0, health=1;
	char*  tok;
	memory *mp;
	struct sembuf sb;
	struct msgbuf buf;
	struct sigaction act;

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
	act.sa_sigaction = *signal_catcher;
	act.sa_flags     = SA_SIGINFO;
	if(sigaction(SIGUSR1,&act,NULL) != 0){
		perror("sig init -- patient");
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
	health = healthCondition(severity,getpid());

	/* Wait for signal */
	while(1){
		while(!signal_rcvd){
			/* Wait for doctor signal and break */
			/* or Increment severity every 1sec */
			health = healthCondition(severity++,getpid());
			if(health == 0)
			{
				printf("Patient Died!");
				return PATIENT_DIED; 
			}
			sleep(1);
		}
		/* Signal Recieved! */
		/* Communicate with Doctor */
		sprintf(buf.mtext,"%d",severity);
		buf.mtype = 1;
		len = strlen(buf.mtext);
		msgqid = findDoctorByPid(signal_pid,mp);
		if (msgsnd(msgqid, &buf, len+1, 0) == -1) /* +1 for '\0' */
			perror("msgsnd -- patient");

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

void signal_catcher(int sig, siginfo_t *si, void *ucontext)
{
	if (sig == SIGUSR1){
		signal_rcvd = 1;
		signal_pid = si->si_pid;
		printf("SIGNAL SENT BY: %d\n"
				"RECIEVED BY:%d\n",signal_pid,getpid());
	}
	return;
}

int findDoctorByPid(pid_t pid,memory* mp){
	for(int i=0; i < NUMBER_OF_DOCTORS; i++){
		if(mp->doctors[i].pid == pid)
		{
			mp->doctors[i].status = 1;
			return mp->doctors[i].msgqid;
		}
	}
	return 0;
}
