#ifndef IPCS_FUNCTIONS
#define IPCS_FUNCTIONS
#define MAX_RETRIES 10

key_t genKey ( char key_id );
int initsem(key_t key, int nsems);
void lock(int semid, struct sembuf *sb, int semnum);
void unlock(int semid, struct sembuf *sb, int semnum);

#endif
