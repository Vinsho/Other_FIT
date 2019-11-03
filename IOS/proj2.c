#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <errno.h>
#include <semaphore.h>
#include <fcntl.h>
#include <zconf.h>
#include <wait.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>

#define INIT_VALUE 0
#define MILI 1000
#define RANDOM_MAX 1000
#define RANDOM_MIN 0
#define BOARDIN_VALUE 100
#define CRASH_WATCH 10000

sem_t *printFr;//semafor na zaistenie vypisov v poradi podla A
sem_t *nastup;//semafor zaistujuci nastup max kapacity do autobusu len pocas pritomnosti busu
sem_t *boardinFreeze;//semafor zaistujucu znemoznenie pristupu na zastavku pocas pritomnosti busu
sem_t *waitBoardin;//semafor zaistujuci pritomnost busu pocas nastupovania
sem_t *finish;//zaistuje vypis finishu u riderov
sem_t *goOn;//caka kym vystupia vsetci ridery




int *CR=NULL; // pocet cakajucich na zastavke
int *A=NULL;//poradove cislo akcie`
int *ridCount=NULL;//pocet riderov
int *finishRidCount=NULL;//pocet ukoncenych riderov

int AId = INIT_VALUE;
int CrId = INIT_VALUE;
int ridId = INIT_VALUE;
int finishRidId = INIT_VALUE;

void initResources();
void rider(int i);
void forkin(int R,int ART);
void bus(int R,int C,int ABT);
void err();
void releaseResources();
void end();
void forkErr();
void argsCheck(char **args);

int main (int argc, char *argv[]){
    FILE *out;
    out= freopen("proj2.out","w",stdout);//presmerovanie stdout do filu proj2.out
    setbuf(stdout, NULL);
    if (argc!=5) err();//pokial nebol zadany ziadany pocet argumentov
    argsCheck(*(&argv));
    int R=strtol(argv[1],NULL,10);//riders
    int C=strtol(argv[2],NULL,10);//kapacita autobusu
    int ART=strtol(argv[3],NULL,10);//max random time pre riderov
    int ABT=strtol(argv[4],NULL,10);//max random time pre bus

    //kontrola 0<ART,ABT<1000

    if(ART<RANDOM_MIN||ART>RANDOM_MAX)err();
    ART*=MILI;//skrz vyuzivanie usleep, potreba vynasobit 1000
    if(ABT<RANDOM_MIN||ABT>RANDOM_MAX)err();
    ABT*=MILI;

    initResources();
    srand(time(NULL));//aktivovanie randomu

    id_t pId = fork();
    if (pId == 0) {
        forkin(R, ART);
        exit(EXIT_SUCCESS);
    }
    else if (pId > 0)//bus
    {
        bus(R,C,ABT);
    }
    else
    {
        forkErr();
    }
    fclose(out);//zatvorenie suboru
    end();
}
//Funkcia sluziaca na aktivovanie semaforov a shared memory
void initResources(){

    unsigned int value = 0;
    printFr = sem_open ("semko1", O_CREAT | O_EXCL, 0644, value+1);
    nastup = sem_open ("semko2", O_CREAT | O_EXCL, 0644, value);
    boardinFreeze = sem_open ("boardinFreeze", O_CREAT | O_EXCL, 0644, value);
    waitBoardin = sem_open ("waitBoardin", O_CREAT | O_EXCL, 0644, value);//bus caka kym nastupuju
    finish = sem_open ("finish", O_CREAT | O_EXCL, 0644, value);
    goOn = sem_open ("goOn", O_CREAT | O_EXCL, 0644, value);//vsetci vystupili a bus moze pokracovat

    sem_unlink ("semko1");
    sem_unlink ("semko2");
    sem_unlink ("boardinFreeze");
    sem_unlink ("waitBoardin");
    sem_unlink ("finish");
    sem_unlink ("goOn");
    AId = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666) ;
    A = shmat(AId, NULL, 0);
    *A=1;
    CrId = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666) ;
    CR = shmat(CrId, NULL, 0);
    *CR=0;
    ridId = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666) ;
    ridCount = shmat(ridId, NULL, 0);
    *ridCount=1;
    finishRidId = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666) ;
    finishRidCount = shmat(finishRidId, NULL, 0);
    *finishRidCount=0;
}
//funkcia sluziaca na upratanie semaforova shared memory
void releaseResources(){
    sem_close(printFr);
    sem_close(nastup);
    sem_close(boardinFreeze);
    sem_close(waitBoardin);
    sem_close(finish);
    sem_close(goOn);

    shmctl(AId, IPC_RMID, NULL);
    shmctl(CrId, IPC_RMID, NULL);
    shmctl(ridId, IPC_RMID, NULL);
    shmctl(finishRidId, IPC_RMID, NULL);
}
// funkcia ovladajuca vypis jednotlivych riderov
void rider(int i)
{
    sem_wait(printFr);
    printf("%d \t\t: RID%d   \t\t: start\n",(*A)++,i);
    sem_post(printFr);
    sem_wait(boardinFreeze);

    printf("%d \t\t: RID%d   \t\t: enter: %d \n",(*A)++,i,++(*CR));
    sem_wait(nastup);
    sem_wait(printFr);
    printf("%d \t\t: RID%d   \t\t: boarding\n",(*A)++,i);
    sem_post(printFr);
    sem_post(waitBoardin);
    sem_wait(finish);

    sem_wait(printFr);
    printf("%d \t\t: RID%d   \t\t: finish\n",(*A)++,i);
    sem_post(printFr);
    (*finishRidCount)++;
    sem_post(goOn);
    end();
}
//funkcia sluziaca na generovanie potrebneho poctu riderov
void forkin(int R,int ART)
{
    id_t p;
    p=fork();
    if (p == 0) //ridergen
    {
        if (*ridCount<=R)rider((*ridCount)++);

    }
    else if (p > 0)
    {
        if(ART!=0)usleep(rand()%ART);
        if (*ridCount<=R)forkin(R,ART);
    }
    else
    {
        forkErr();
    }
    end();

}
//funkcia sluziaca na vypis BUSu
void bus(int R,int C,int ABT)
{
    int nastupili=0;
    int value=0;
    while(*finishRidCount!=R && *A<CRASH_WATCH )
    {

        sem_wait(printFr);
        printf("%d \t\t: BUS   \t\t: start\n",(*A)++);
        printf("%d \t\t: BUS   \t\t: arrival\n",(*A)++);
        sem_post(printFr);

        sem_getvalue(boardinFreeze,&value);
        while(value>0)//aktivuje boardinFreeze, riders nemozu vchadzat na zastavku
        {
            sem_wait(boardinFreeze);
            sem_getvalue(boardinFreeze,&value);
        }

        if(*CR!=0) {//pokial niekto caka na zastavke
            sem_wait(printFr);
            printf("%d \t\t: BUS   \t\t: start boarding: %d\n", (*A)++, *CR);
            sem_post(printFr);
            nastupili=*CR;
            if (nastupili>C){
                nastupili=C;
            }
            *CR=*CR-nastupili;

            for(int i=0;i<nastupili;i++){//povoli nastup vodicom
                sem_post(nastup);
                sem_wait(waitBoardin);
            }

            sem_wait(printFr);
            printf("%d \t\t: BUS   \t\t: end boarding: %d\n", (*A)++, *CR);
            sem_post(printFr);

        }

        sem_wait(printFr);
        printf("%d \t\t: BUS   \t\t: depart\n",(*A)++);
        sem_post(printFr);

        for(int i=0;i<BOARDIN_VALUE;i++) {//umozni pristup na zastavku
            sem_post(boardinFreeze);
        }

        if(ABT!=0)usleep(rand()%ABT);//uspi bus na upresnenu random dobu
        sem_wait(printFr);
        printf("%d \t\t: BUS   \t\t: end\n",(*A)++);
        sem_post(printFr);

        while(nastupili>0)//pre vsetkych riderov ktory nastupili do busu
        {
            sem_post(finish);//aktivuje finish u riderov
            nastupili--;
            sem_wait(goOn);

        }

    }
    printf("%d \t\t: BUS   \t\t: finish\n",(*A)++);

}
void argsCheck(char **args){
    for (int i=1;i<5;i++)
    {
            if (!isdigit(args[i][0])) err();
    }
}
//funkcia na uvolnenie alokovanej pamate a uspesne ukoncenie
void end(){
    releaseResources();
    exit(EXIT_SUCCESS);
}
//funkcie na vypis errorov
void err(){
    fprintf(stderr,"Chybne argumenty spustenia\n");
    releaseResources();
    exit(EXIT_FAILURE);
}

void forkErr()
{
    fprintf(stderr,"Chybny fork\n");
    releaseResources();
    exit(EXIT_FAILURE);
}

