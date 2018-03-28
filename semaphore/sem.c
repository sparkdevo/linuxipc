#include <sys/types.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define SHMDATASIZE 1000
#define BUFFERSIZE (SHMDATASIZE-sizeof(int))
#define SN_EMPTY 0
#define SN_FULL 1
int deleteSemid=0;

void server(void);
void client(int shmid, int semid);
void delete(void);
void sigdelete(int signum);
void locksem(int signum, int semnum);
void unlocksem(int semid, int semnum);
void waitzero(int semid, int semnum);
void clientwrite(int shmid, int semid, char *buffer);

union semun
{
        int val;
        struct semid_ds *buf;
        ushort *array; // cmd == SETALL，或 cmd = GETALL
};
int safesemget(key_t key, int nssems, int semflg);
int safesemctl(int semid, int semunm, int cmd, union semun arg);
int safesemop(int semid, struct sembuf *sops, unsigned nsops);
int safeshmget(key_t key, int size, int shmflg);
void *safeshmat(int shmid, const void *shmaddr, int shmflg);
int safeshmctl(int shmid, int cmd, struct shmid_ds *buf);

int main(int argc, char *argv[ ])
{
    if(argc < 2){
        server();
    }
    else{
        client(atoi(argv[1]), atoi(argv[2]));
    }
    return 0;
}

void server(void)
{
    union semun sunion;
    int semid, shmid;
    //void *shmdata;
    char *buffer;
    //shmdata = NULL;
    semid = safesemget(IPC_PRIVATE, 2, SHM_R|SHM_W);
    deleteSemid = semid;
    atexit(&delete);
    signal(SIGINT, &sigdelete);
    sunion.val = 1;
    safesemctl(semid, SN_EMPTY, SETVAL, sunion);
    sunion.val = 0;
    safesemctl(semid, SN_FULL, SETVAL, sunion);
    shmid = safeshmget(IPC_PRIVATE, SHMDATASIZE, IPC_CREAT|SHM_R|SHM_W);
    //*(int *)shmdata = safeshmat(shmid, 0, 0);
    buffer = safeshmat(shmid, 0, 0);
    safeshmctl(shmid, IPC_RMID, NULL);
    //*(int *)shmdata = semid;
    //buffer = shmdata + sizeof(int);
    printf("Server is running with SHM id ** %d**\n", shmid);
    printf("Server is running with SEM id ** %d**\n", semid);
    while(1)
    {
        printf("Waiting until full...");
        fflush(stdout);
        locksem(semid, SN_FULL);
        printf("done.\n");
        printf("Message received: %s.\n", buffer);
        unlocksem(semid, SN_EMPTY);
    }
}

void client(int shmid, int semid)
{
    //int semid;
    //void *shmdata;
    char *buffer;
    //shmdata = NULL;
    buffer = safeshmat(shmid, 0, 0);
    //semid = *(int*)shmdata;
    //buffer = shmdata;
    printf("Client operational: shm id is %d, sem id is %d\n", shmid, semid);
    while(1)
    {
        char input[3];
        printf("\n\nMenu\n1.Send a message\n");
        printf("2.Exit\n");
        fgets(input, sizeof(input), stdin);
        switch(input[0])
        {
            case '1':
                clientwrite(shmid, semid, buffer);
                break;
            case '2':
                exit(0);
                break;
        }
    }
}

void delete(void)
{
    printf("\nMaster exiting; deleting semaphore %d.\n", deleteSemid);
    if(semctl(deleteSemid, 0, IPC_RMID, 0)==-1){
        printf("Error releasing semaphore.\n");
    }
}

void sigdelete(int signum)
{
    exit(0);
}

void locksem(int semid, int semnum)
{
    struct sembuf sb;
    sb.sem_num = semnum;
    sb.sem_op = -1;
    sb.sem_flg = SEM_UNDO;
    safesemop(semid, &sb, 1);
}

void unlocksem(int semid, int semnum)
{
    struct sembuf sb;
    sb.sem_num = semnum;
    sb.sem_op = 1;
    sb.sem_flg = SEM_UNDO;
    safesemop(semid, &sb, 1);
}

void waitzero(int semid, int semnum)
{
    struct sembuf sb;
    sb.sem_num = semnum;
    sb.sem_op = 0;
    sb.sem_flg = 0;
    safesemop(semid, &sb, 1);
}

void clientwrite(int shmid, int semid, char *buffer)
{
    printf("Waiting until empty...");
    fflush(stdout);
    locksem(semid, SN_EMPTY);
    printf("done.\n");
    printf("Enter Message: ");
    fgets(buffer, SHMDATASIZE, stdin);
    unlocksem(semid, SN_FULL);
}

int safesemget(key_t key, int nsems, int semflg)
{
    int retval;
    if((retval = semget(key, nsems, semflg)) == -1){
        printf("semget error: %s.\n", strerror(errno));
        exit(254);
    }
    return retval;
}

int safesemctl(int semid, int semnum, int cmd, union semun arg)
{
    int retval;
    if((retval = semctl(semid, semnum, cmd, arg)) == -1)
    {
        printf("semctl error: %s.\n", strerror(errno));
        exit(254);
    }
    return retval;
}

int safesemop(int semid, struct sembuf *sops, unsigned nsops)
{
    int retval;
    if((retval = semop(semid, sops, nsops)) == -1)
    {
        printf("semop error: %s.\n", strerror(errno));
        exit(254);
    }
    return retval;
}

int safeshmget(key_t key, int size, int shmflg)
{
    int retval;
    if((retval = shmget(key, size, shmflg)) == -1)
    {
        printf("shmget error: %s.\n", strerror(errno));
        exit(254);
    }
    return retval;
}

void *safeshmat(int shmid, const void *shmaddr, int shmflg)
{
    return shmat(shmid,shmaddr,shmflg);
    /*int retval;
    if((retval = ) == -1)
    {
        printf("shmat error: %s.\n", strerror(errno));
        exit(254);
    }
    return retval;*/
}

int safeshmctl(int shmid, int cmd, struct shmid_ds *buf)
{
    int retval;
    if((retval = shmctl(shmid, cmd, buf)) == -1)
    {
        printf("shmctl error: %s.\n", strerror(errno));
        exit(254);
    }
    return retval;
}