#include <stdio.h>
#include <stdbool.h>
#include <memory.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_ADRESY 42//maximalny pocet adries
#define MAX_DLZKA 100//maximalna dlzka adresy
#define SPACE 32 //hodnota medzery v ascii

int getAdresy(char *prvePismena,char adresy[MAX_ADRESY][MAX_DLZKA]);
int getZnaky(char c, int b,int poradie,char adresy[MAX_ADRESY][MAX_DLZKA]);
char *abecedne(int chCount,char nextChar[]);
int result(int chCount,int addressCount,char nextChar[],char *found);
int unique(int chCount,char *nextChar,int i,int poradie,char adresy[MAX_ADRESY][MAX_DLZKA]);

int main(int argc,char *argv[]) {
    char prvePismena[MAX_ADRESY];
    char adresy[MAX_ADRESY][MAX_DLZKA];
    int chCount=0;
    int poradie = 1; // miesto na ktorom sa znak v adrese nachadza
    getAdresy(prvePismena,adresy);
    if(argc>2) //ked zadal viac ako 1 argument
    {
        fprintf(stderr,"Nadmerny pocet argumentov!");
        return EXIT_FAILURE;
    }
    if (argc<2) // pokial nie je zadany argument tak vypise prve pismena moznych slov
    {
        for (int i=1;i<MAX_ADRESY;i++) chCount=unique(chCount,prvePismena,i,0,adresy);
        printf("FOUND: %s",prvePismena);
        return 0;
    }

    else
        while (argv[1][poradie-1]!='\0') poradie=getZnaky(argv[1][poradie-1],argv[1][poradie],poradie,adresy);//kym dalsie pismeno v argumente nie je prazdny znak tak opakuje funkciu getZnaky
}

int getAdresy(char *prvePismena,char adresy[MAX_ADRESY][MAX_DLZKA]) // nacita adresy do arrayu
{
    int chCount=0;
    for (int i=0;i<MAX_ADRESY;i++)
    {
        chCount++;
        char c;
        int j=0;
        while((c=getchar())!='\n'&& c!=EOF) {
                adresy[i][j] = c;
                adresy[i][j] = toupper(adresy[i][j]);
                if (c>=0&&c<=31) //31 je horna hranica netlacitelnych znakov
                {//pokial je v subore neplatny znak
                    fprintf(stderr, "Znak '%c' v subore s adresami je nepovoleny,nacadza sa v %i. riadku na %i. mieste!!", adresy[i][j], i+1, j+1);
                    exit(EXIT_FAILURE);
                }
                j++;
                if (j>100) {
                    fprintf(stderr,"Adresa dlhsia ako 100 znakov!");
                    exit(EXIT_FAILURE);
                }
            }
        adresy[i][j]='\0';
        prvePismena[i]=adresy[i][0];
    }
    return 0;
}

char *abecedne(int chCount,char *nextChar)// zoradi znaky abecedne
{
    char pom;
    for (int i=0;i<chCount;i++)
    {
        for (int j=0;j<chCount-1-i;j++)
        {
            if (nextChar[j]>nextChar[j+1])
            {
                pom=nextChar[j+1];
                nextChar[j+1]=nextChar[j];
                nextChar[j]=pom;
            }
        }
    }
    nextChar[chCount]='\0';// ukonci array znakov
    return nextChar;
}
int unique(int chCount,char *nextChar,int i,int poradie,char adresy[MAX_ADRESY][MAX_DLZKA]) // funkcia overuje ci neni nejaky znak duplicitny
{
    bool New = true;
    for (int j = 0; j <= chCount; j++)
        if (adresy[i][poradie] == nextChar[j])  //pokial sa rovna co i len jednemu tak nie je zaradeny
        {
            New = false;
            break;
        }
    if (New) {  // pokial sa nicomu nerovnal tak bol zaradeny do arrayu vyslednych znakov
        nextChar[chCount] = adresy[i][poradie];
        chCount++;
    }
    nextChar=abecedne(chCount,nextChar);
    return chCount;
}

int getZnaky(char c,int b,int poradie,char adresy[MAX_ADRESY][MAX_DLZKA]) // ziska dalsie znaky
{
    int chCount = 0; // pocet moznych dalsich znakov
    char nextChar[25];// array moznych znakov
    char *found;
    int addressCount = 0;
    c = toupper(c);
    if (c>=0&&c<=31)// 31 je horna hranica netlacitelnych znakov
    if (c!='\n'){ //pokial je v argumente neplatny znak
        fprintf(stderr, "Nepovoleny znak!!");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < MAX_ADRESY; i++) {
        if (c == adresy[i][poradie - 1]) {
            found = adresy[i];
            addressCount++;//rata kolko slov sa nachadza medzi vyhovujucimi
                if (adresy[i][poradie] !='\0') {//zaruci aby to bol platny znak
                    chCount=unique(chCount,nextChar,i,poradie,adresy);
                }
                else printf("FOUND: %s\n", adresy[i]); // vypise slovo v pripade ze tvori zaklad dalsieho slova, ktoremu este mozme hladat dalsie znaky
        }
        else
            for (int j = 0; j < MAX_DLZKA; j++) memset(adresy[i], 0, MAX_DLZKA);//vymaze adresu ktora uz nie je pouzitelna

    }

        if (b=='\0')result(chCount, addressCount, nextChar, found);// pokial dalsi znak v argumente je = '\0' povola funkciu result
        poradie ++;
        return poradie;
}


int result(int chCount,int addressCount,char nextChar[],char *found)// vypise finalny vysledok
{
    if(chCount>=1&&addressCount>1) { //vypise ENABLED znaky
            printf("ENABLED: ");
            printf("%s", nextChar);

        return 0;
    }
    if (chCount==1&&addressCount==1){
        printf("FOUND: "); //vypise FOUND slovo
        printf("%s", found);
    }
    if(chCount==0) { // pokial je pocet vyhovujucich znakov nula tak vypise not found
        printf("Not found");
    }
 return 0;
}
