#include "local.h"
#include "ipc_functions.h"
#include "queue.h"

int initDrCommunication(int msgqid, struct msgbuf* buf);
int schedulePatient(int msgqid, struct msgbuf* buf);
void treatPatient(int msqid, struct msgbuf* buf);
void acquirePatient(memory* mp, queue* q);
void makeMeAvailable(memory* mp);
void signal_catcher(int sig);

int signal_rcvd = 0;

int main ( int argc, char *argv[] )
{
	int           shmid,semid,msgqid,slept=0;
	char*         tok;
	memory        *mp;
	key_t         key;
	struct sembuf sb;
	struct msgbuf buf;
	struct sigaction act;

	/* For some reason arguments are concatenated in argv1*/
	tok   = strtok(argv[1]," ");
	shmid = atoi(tok);
	tok   = strtok(NULL," ");
	semid = atoi(tok);

	if(argc != 2)
	{
		printf("Usage ./doctor [memory id] [semaphore id]\n");
		return EXIT_FAILURE;
	}

	/* Attach Shared-Memory*/
	if( ( mp = shmat(shmid, NULL, 0) ) == (void*) -1)
	{
		perror("doctor -- shmat");
		return EXIT_FAILURE;
	}

	/* Change signal displacement */
	act.sa_sigaction = *signal_catcher;
	if(sigaction(SIGTERM,&act,NULL) != 0){
		perror("sig init -- doc");
		return EXIT_FAILURE;
	}

	/* Generate Message Q Key & Create */
	key = genKey(getpid());
	if ((msgqid = msgget(key, 0666 | IPC_CREAT)) == -1) {
		perror("msgget");
		return EXIT_FAILURE;
	}

	/* Push doctor msqid to Shared-Memory */
	sb.sem_flg = SEM_UNDO;
	lock(semid,&sb,0);
	mp->doctors[mp->doctorCount].pid      = getpid(); /* So patient can find it */
	mp->doctors[mp->doctorCount++].msgqid = msgqid;
	unlock(semid,&sb,0);

	sleep(5);                              /* Patients head-start */

	/* Doctor's Work :3*/
	while(!signal_rcvd && slept < 3){
		lock(semid,&sb,1);
		if( !isEmpty( &(mp->patientQueue) ) ){
			int n;
			acquirePatient(mp, &(mp->patientQueue) );
			unlock(semid,&sb,1);
			/* Recieve Symptoms */
			if ((n = initDrCommunication(msgqid, &buf)) != -1){          /* rcv imsick */
				sleep(DOCTOR_SLEEP_TIME);
				if(strncmp(buf.mtext,"imsick",n) ==  0){
					schedulePatient(msgqid, &buf);               /* Send show-up */
					treatPatient(msgqid, &buf);                  /* Treat until recoved */
					makeMeAvailable(mp);                         /* Look for another patient */
				}
			}
			signal_rcvd = 0;
			slept       = 0;
		}
		else
		{
			printf("%d:Queue is empty, zZzZz\n",getpid());
			unlock(semid,&sb,1);
			slept++;
			sleep(DOCTOR_SLEEP_TIME);
		}
	}

	/* Delete Stuff */
	if (msgctl(msgqid, IPC_RMID, NULL) == -1) {
		perror("doctor -- msgrm");
		return EXIT_FAILURE;
	}
	if( shmdt(mp) == -1)
	{
		perror("doctor -- shmem detach");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int initDrCommunication(int msgqid, struct msgbuf* buf){
	int res;
	if( (res = msgrcv(msgqid,buf, sizeof(buf->mtext), 0, 0)) == -1){
		perror("doctor -- msginit");
	}
	return res;
}
int schedulePatient(int msgqid, struct msgbuf* buf){
	int res,len;
	memset(&(buf->mtext),0,BUFF_SIZE*sizeof(char));
	strcpy(buf->mtext,"show-up");
	buf->mtype = 1;
	len = strlen(buf->mtext);
	res =  msgsnd(msgqid, buf, len+1, 0);
	if(res == -1)
	{
		perror("patient -- msg init");
		exit(EXIT_FAILURE);
	}
	return res;
}
void treatPatient(int msgqid, struct msgbuf* buf){
	int len,localSeverity;
	do{
		if( msgrcv(msgqid,buf, sizeof(buf->mtext), 0, 0) == -1){
			perror("doctor -- treatmsginit");
			printf("%d\n",errno);
			exit(EXIT_FAILURE);
		}
		localSeverity = atoi(buf->mtext);
		localSeverity--;
		memset(&(buf->mtext),0,BUFF_SIZE*sizeof(char));
		sprintf(buf->mtext,"%d",localSeverity);
		buf->mtype = 1;
		len = strlen(buf->mtext);
		if(msgsnd(msgqid, buf, len+1, 0) == -1)
		{
			perror("patient -- sendseverity");
			exit(EXIT_FAILURE);
		}
	}while(localSeverity);
}

void acquirePatient(memory* mp, queue* q){
	int myPatient;
	while( (myPatient = dequeue(q)) == -1);
	kill(myPatient,SIGUSR1);
}

void makeMeAvailable(memory* mp){
	pid_t myPid = getpid();
	for(int i=0; i<NUMBER_OF_DOCTORS;i++){
		if(mp->doctors[i].pid == myPid)
		{
			mp->doctors[i].myPatient = -1;
			break;
		}
	}
}

void signal_catcher(int sig)
{
	if (sig == SIGTERM){
		signal_rcvd = 1;
		printf("Doc %d Terminated\n",getpid());
	}
	return;
}
