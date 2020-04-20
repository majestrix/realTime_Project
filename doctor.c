#include "local.h"
#include "ipc_functions.h"
#include "queue.h"

int initDrCommunication(int msgqid, struct msgbuf* buf);
int schedulePatient(int msgqid, struct msgbuf* buf);
void treatPatient(int msqid, struct msgbuf* buf);

int main ( int argc, char *argv[] )
{
	int           shmid,semid,msgqid;
	char*         tok;
	memory        *mp;
	key_t         key;
	struct sembuf sb;
	struct msgbuf buf;

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

	/* Doctor's Work :3*/
	while(1){
		if( !isEmpty( &(mp->patientQueue) ) ){
			int n;
			lock(semid,&sb,1);
			kill(removeData(&(mp->patientQueue)),SIGUSR1);
			unlock(semid,&sb,1);
			/* Recieve Symptoms */
			if ((n = initDrCommunication(msgqid, &buf)) != -1){ /* rcv imsick */
				if(strncmp(buf.mtext,"imsick",n) == 0){
					schedulePatient(msgqid, &buf); /* Send show-up */
					treatPatient(msgqid, &buf);	
					break;
				}
			}
		}
//		else{
//			printf("%d:Queue is empty, zZzZz\n",getpid());
////			sleep(DOCTOR_SLEEP_TIME);
//		}
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
			perror("doctor -- msginit");
			exit(EXIT_FAILURE);
		}
		printf("%s\n",buf->mtext);
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
