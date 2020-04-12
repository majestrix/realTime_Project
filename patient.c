#include "local.h"
#include "ipc_functions.h"
#include "queue.h"
/* #define COVID_19_FEVER    0
 * #define COVID_19_COUGH    0
 * #define COVID_19_BREATH   0
 * #define BLOOD_HYPER       0
 * #define HEART_RESP        0
 * #define PATIENT_AGE       0
 */
int generateNumber (int low, int high);
int isSevere(int num);

int main ( int argc, char *argv[] )
{
	int           shmid,semid,len;
	char*         str;
	memory        *mp;
	struct sembuf sb;

	int fever  = isSevere( generateNumber(0,8) );
	int cough  = isSevere( generateNumber(0,8) );
	int breath = isSevere( generateNumber(0,8) );
	int hyper  = isSevere( generateNumber(0,8) );
	int heart  = isSevere( generateNumber(0,8) );
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

	/* Patient code here*/
	sb.sem_flg = SEM_UNDO;
	lock(semid,&sb,getpid()%10);
	insert( &( mp->patientQueue ),getpid() );
	unlock(semid,&sb,getpid()%10);
	
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
