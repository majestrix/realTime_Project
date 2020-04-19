#include "local.h"
#include "ipc_functions.h"
#include "queue.h"

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
			lock(semid,&sb,1);
			kill(removeData(&(mp->patientQueue)),SIGUSR1);
			unlock(semid,&sb,1);
			/* Recieve Symptoms */
			printf("Rcv from patient\n");
			if (msgrcv(msgqid, &buf, sizeof(buf.mtext), 0, 0) == -1) {
				perror("msgrcv");
				return EXIT_FAILURE;
			}
			printf("\nseverity received is, %d \n",atoi(buf.mtext));
			if (atoi(buf.mtext) > 15){
				printf("\nPatient's case is hopeless...\n");
//				kill(removeData(&(mp->patientQueue)),SIGKILL);
			}
			else {
				printf("\nPatient is recovered...\n");
				break;
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

