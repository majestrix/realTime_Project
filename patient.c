#include "local.h"
#include "ipc_functions.h"
#include "queue.h"

int generateNumber (int low, int high);
int isSevere(int num);
int healthCondition(int s, int pid);
int healthSeverity(int fever,int cough,int breath ,int hyper,int heart,int cancer);
int ageSeverity(int age);
void signal_catcher(int sig, siginfo_t *si, void *ucontext);
//void hopeless_signal(int sig);
int findDoctorByPid(pid_t pid,memory* mp);
int initDrCommunication(int msgqid, struct msgbuf* buf);
int waitForDr(int msgqid, struct msgbuf *buf);
int acceptTreatment(int msgqid, struct msgbuf* buf,int severity);

int   signal_rcvd = 0;
pid_t signal_pid;

int main ( int argc, char *argv[] )
{
	int    shmid,semid,msgqid;
	int    severity = 0, status=1;
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
		perror("sig init1 -- patient");
		return EXIT_FAILURE;
	}
//	if(sigset(SIGUSR2,hopeless_signal) == -1){
//		perror("sig init2 -- patient");
//		return EXIT_FAILURE;
//	}
	/* Enqueue patient */
	sb.sem_flg = SEM_UNDO;
	lock(semid,&sb,1);
	insert(&(mp->patientQueue) ,getpid() ); 
	unlock(semid,&sb,1);

	/* Initial severity */
	severity  = healthSeverity(fever,cough,breath,hyper,heart,cancer);
	severity += ageSeverity(age);
	status = healthCondition(severity,getpid());

	/* Wait for signal */
	if(status == PATIENT_DIED)
	{
		while(1){
			while(!signal_rcvd){
				/* Wait for doctor signal and break */
				/* or Increment severity every 1sec */
				status = healthCondition(severity++,getpid());
				if(status == 0)
				{
					printf("P:%d Died! -- Waited too long.\n",getpid());
					status = PATIENT_DIED; 
				}
				sleep(1);
			}
			/* Signal Recieved! */
			/* Communicate with Doctor */
			msgqid = findDoctorByPid(signal_pid,mp);
			if( initDrCommunication(msgqid,&buf) != -1 ) /* Sends imsick */
			{
				/* die if Doctor took too long*/
				if( (status = waitForDr(msgqid,&buf)) == PATIENT_DIED) /* Waits for show-up */
					break;
				else
				{
					if( acceptTreatment(msgqid,&buf,severity) == 0 ){
						printf("P:%d Recovered!\n",(int)getpid());
						status = PATIENT_RECOVERED;
						break;
					}
				}
			}
		}
	}

	if( shmdt(mp) == -1)
	{
		perror("doctor -- shmem detach");
		return EXIT_FAILURE;
	}

	return status;
}

int initDrCommunication(int msgqid, struct msgbuf* buf){
	int res,len;
	memset(&(buf->mtext),0,BUFF_SIZE*sizeof(char));
	strcpy(buf->mtext,"imsick");
	buf->mtype = 1;
	len = strlen(buf->mtext);
	res =  msgsnd(msgqid, buf, len+1, 0);
	if(res == -1)
	{
		perror("patient -- msg init");
		exit(EXIT_FAILURE);
	}
	printf("P:%d->D%d: imsick\n",getpid(),signal_pid);
	return res;
}

/* non-blocking msgrcv repeats for defined PATIENT_WAIT_TIME*/
int waitForDr(int msgqid, struct msgbuf *buf){
	int terminate=0;
	time_t start,end;
	double elapsed;
	start = time(NULL);
	while(!terminate)
	{
		end = time(NULL);	
		elapsed = difftime(end,start);
		if(elapsed < PATIENT_WAIT_TIME)
		{
			if (msgrcv(msgqid, buf, sizeof(buf->mtext), 0, IPC_NOWAIT) != -1) {
				terminate = 1;
				printf("D:%d->P%d: %s\n",(int)signal_pid,(int)getpid(),"show-up");
				return 1;
			}
		}
		else
		{
			return PATIENT_DIED;
		}
	}
	return 0;
}

/* Bounce and decrement severity until 0*/
int acceptTreatment(int msgqid, struct msgbuf* buf,int severity){
	int len,localSeverity=severity;
	while(localSeverity){
		memset(&(buf->mtext),0,BUFF_SIZE*sizeof(char));
		sprintf(buf->mtext,"%d",localSeverity);
		buf->mtype = 1;
		len = strlen(buf->mtext);
		if(msgsnd(msgqid, buf, len+1, 0) == -1)
		{
			perror("patient -- sendseverity");
			exit(EXIT_FAILURE);
		}
		if( msgrcv(msgqid,buf, sizeof(buf->mtext), 0, 0) == -1){
			perror("doctor -- msginit");
			exit(EXIT_FAILURE);
		}
		localSeverity = atoi(buf->mtext);;
	}
	return localSeverity;
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
		return PATIENT_DIED;
	}
	return 0;
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

void hopeless_signal(int sig){
	if(sig == SIGUSR2)
	{
		exit(PATIENT_DIED);
	}
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
