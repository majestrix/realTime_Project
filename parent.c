#include "local.h"
#include "queue.h"
#include "ipc_functions.h"
#include <pthread.h>

void* forker_function(void *args);
void* thread_print_stats(void *args);

typedef struct targs_struct {
	pid_t* patients;
	int* patientsNum;
	int shmid;
	int semid;
} targs; 

int terminate=0;
int patientsNum=0,deaths=0,recovers=0;
pid_t     patients[MAX_Q];

int main ( int argc, char *argv[] )
{

	int              shmid,semid,i;
	int              status=0;
	key_t            key;
	pid_t            doctors[NUMBER_OF_DOCTORS];
	pthread_t        forker;
	pthread_t        printer;
	targs            *argCombo;
	memory           *mp;

	/*-----------------------------------------------------------------------------
	 *  SHARED MEMORY INIT
	 *-----------------------------------------------------------------------------*/

	if( (shmid = shmget(0x1234, sizeof(memory), 0666|IPC_CREAT)) == -1)
	{
		perror("parent -- shmgt");
		return EXIT_FAILURE;
	}

	if( ( mp = shmat(shmid, NULL, 0) ) == (void*) -1)
	{
		perror("parent -- shmat");
		return EXIT_FAILURE;
	}

	/*-----------------------------------------------------------------------------
	 *  SEMAPHORE INIT
	 *-----------------------------------------------------------------------------*/
	/* two semaphores */
	key   = genKey('A');                    /* atomic read&write */
	semid = initsem(key, 2);                /* for both doctor&patient */

	/*-----------------------------------------------------------------------------
	 *  INIT FORKS & Q
	 *-----------------------------------------------------------------------------*/

	initQueue(&(mp->patientQueue));         /* Queue implementation */

	for(i = 0; i < NUMBER_OF_DOCTORS; i++){ /* Fork doctors */

		if( (doctors[i] = fork() ) == -1){
			perror("parent -- fork doctor");
			return EXIT_FAILURE;
		}else if(doctors[i] == 0){
			char argtxt[15]={0};
			sprintf(argtxt,"%d %d",shmid,semid);
			execlp("./doctor","./doctor",argtxt,(char*)NULL);
			perror("parent -- exec doctor");
			return EXIT_FAILURE;
		}
	}

	argCombo = (targs*) malloc(sizeof(targs));
	argCombo->shmid = shmid;
	argCombo->semid = semid;
	pthread_create(&forker, NULL, forker_function, argCombo);
	pthread_create(&printer, NULL, thread_print_stats, NULL);
	/*-----------------------------------------------------------------------------
	 *  PARENT WORK
	 *-----------------------------------------------------------------------------*/

	/* Monitor exit statuses*/

	i = 0;
	while(!terminate){
		pid_t wpid = waitpid(-1, &status, WNOHANG);
		if(wpid > 0 && WIFEXITED(status))
		{
			int cond = WEXITSTATUS(status);
			switch(cond)
			{
				case PATIENT_DIED:
					deaths++;
					break;
				case PATIENT_RECOVERED:
					recovers++;
					break;
				default:
					break;
			}
		}
		if(deaths >= THRESHOLD)
		{
			printf("Termination condition achieved!\n");
			printf("\033[0;36mPatients:%d, Recovered:%d, Dead:%d\033[0m\n",patientsNum,recovers,deaths);
			terminate = 1;
		}
	}

	printf ( "======================================================\n" );
	printf("Cleaning up!\n");

	/* Retire Doctors & Terminate Remaining/Stuck Patients*/
	pid_t patientPid;
	while( (patientPid = dequeue( &( mp->patientQueue))) != -1){
		kill(patientPid,SIGKILL);
	}
	for(i = 0 ; i < NUMBER_OF_DOCTORS; i++){
		pid_t docPid = mp->doctors[i].pid;
		pid_t patientPid = mp->doctors[i].myPatient;
		if(kill(docPid,SIGKILL) == 0){
			msgctl(mp->doctors[i].msgqid, IPC_RMID, NULL);
			printf("Terminated Doc %d\n",mp->doctors[i].pid);
		}
		if( patientPid != -1)
			kill(patientPid,SIGKILL);
	}

	printf ( "======================================================\n" );
	/* Print final results*/
	pthread_join(forker,NULL);
	pthread_join(printer,NULL);
	while(wait(&status) > 0);
	printf("\033[0;33mFinal Results:\nPatients:%d, Recovered:%d, Dead:%d\033[0m\n",patientsNum,recovers,deaths);

//	printf("Contents of sharedmemory:\n");
//	printShmem(mp);

	/*-----------------------------------------------------------------------------
	 *  Delete IPCS
	 *-----------------------------------------------------------------------------*/

	if(shmctl(shmid, IPC_RMID, (struct shmid_ds *) 0) == -1 ){
		perror("parent -- remove shm");
		return EXIT_FAILURE;
	}
	if(semctl(semid,0, IPC_RMID)  == -1 ){
		perror("parent -- remove sem");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

void* forker_function(void *args){
	while(!terminate && patientsNum < MAX_Q){
		if((patients[patientsNum++] = fork()) == -1){
			perror("parent -- fork patient");
			break;
		}else if(patients[patientsNum-1] == 0){
			char argtxt[15]={0};
			sprintf(argtxt,"%d %d", ((targs*)args)->shmid, ((targs*)args)->semid);
			execlp("./patient","./patient",argtxt,(char*)NULL);
			perror("parent -- exec patient");
			break;
		}
		sleep(FORK_NEW_PATIENT);
	}
	return NULL;
}

void* thread_print_stats(void *args){
	sleep(10);
	while(!terminate){
		printf("\033[0;36mPatients:%d, Recovered:%d, Dead:%d\033[0m\n",patientsNum,recovers,deaths);
		sleep(5);
	}
	return NULL;
}


