#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#define IT_COUNT 10 //dostatocny pocet iteracii pri cfrac na 10 desatinnych miest
#define MAX_VYSKA 100 //maximalna povolena vyska meracieho zariadenia
#define MAX_UHOL 1.4 //maximalna velkost uhlu A a B
#define MAX_IT 13 //maximalmy povoleny pocet interacii
#define MIN 0

double taylor_tan(double x, unsigned int n);
bool tang (char *argv[]);
double cfrac_tan(double x, unsigned int n);
bool procesor(char *argv[],int argc);
int errors(int err);
bool help();

int main(int argc, char *argv[]) {
    bool go = false; // premenna do ktorej
    if (argc<2||argc>6) errors(6); // pokial nezadal ziaden argument tak vypise chybove hlasenie

    if (strcmp(argv[1],"--tan")==0&&argc==5) go=tang(argv);// pokial bol program spusteny s argumentom --tan
    else if (strcmp(argv[1],"-c")==0||(strcmp(argv[1],"-m")==0 && argc <5)) go=procesor(argv,argc);// pokial bol program spusteny s argumentom --c
    else if  (strcmp(argv[1],"--help")==0&&argc==2) go=help();// pokial bol program spusteny s argumentom --help
    if (go)//go je nastavene na true pokial bol program spusteny so spravnymi argumentami pokial nie, vypise sa prikaz na help
    {
        return EXIT_SUCCESS;
    }
    else errors(6);

    return 0;

}

bool help() //vypise help
{
    printf("--------------||Funkcia '--tan A N M'||-------------\n"
                   "Porovna presnost vypoctu tangens uhlu A (zadaneho v radianoch) medzi tan z matematickej kniznice a vypoctom tangensu pomocou Taylorovho polynomu a zretazenych zlomkov.\n"
                   "Argumenty N a M udavaju, v ktorych iteraciach iteracneho vypoctu ma porovnavanie prebiehat. 0 < N <= M < 14.\n");
    printf("Vystup je podavany vo forme I M T TE C CE, kde:\n"
                   "->'I' znaci pocet iteracii iteracneho vypoctu,\n"
                   "->'M' je vysledok funkcie tan z matematickej kniznice,\n"
                   "->'T' je vysledok vypoctu tangensu pomocou taylorovho polynomu,\n"
                   "->'TE' je absolutna chyba medzi M a T,\n"
                   "->'C' je vysledok vypoctu tangensu pomocou zretazenych zlomkov,\n"
                   "->'CE' je absolutna chyba medzi C a M\n");
    printf("-------------||Funkcia '[-c X] -m A [B]'--------------\n"
                   "Umoznuje meranie vzdialenosti a vysky vzdialenych objektov\n");
    printf("->Argument -c nastavuje vysku meriaceho pristroju pre vypocet . Vyska je dana argumentom X(0 < X <= 100). Argument je volitelny - implicitna vyska je 1.5 metra.\n"
                   "->Uhol α je dany argumentom A v radianoch. 0 < A <= 1.4 < π/2\n"
                   "  α udava uhol medzi vodorovnou priamkou vychadzajucou z vysky meriaceho pristroju a priamkou smerujucou ku podstave objektu.\n"
                   "->Pokial je zadany aj uhol β, tak program vypocita aj vysku objektu. Je udavany argumentom B. 0 < B <= 1.4 < π/2\n"
                   "  β udava uhol medzi vodorovnou priamkou vychadzajucou z vysky meriaceho pristroju a primkou smerujucou ku vrcholu objektu.");

    return true;
}

bool tang (char *argv[]) //funkcia vypocita Tang 3 sposobmi, a relativne odchylky medzi nimi
{
    double a = strtod(argv[2], NULL); // uhol v radianoch
    long n = strtol(argv[3], NULL, 10) ;//zaciatocna iteracia
    long m = strtol(argv[4], NULL, 10) ;//konecna iteracia
    if ((n<1||n>m)||m>MAX_IT) errors(0);

    double TE ;//absolutna chyba medzi tan z kniznice a tan vypocitanym pomocou taylorovho radu
    double CE;//absolutna chyba medzi tan z kniznice a tan vypocitanym pomocou continued fraction
    for(long i=n;i<=m;i++) {
        TE=fabs(taylor_tan(a,i)-tan(a));
        CE=fabs(cfrac_tan(a,i)-tan(a));

        printf("%ld. %e %e %e %e %e\n", i, tan(a), taylor_tan(a, i),TE, cfrac_tan(a, i),CE);
    }
    return true;
}

double taylor_tan(double x, unsigned int n) //vypocitanie tangensu pomocou taylorovho radu
{
    double tan=0;
    double koef=x;
    unsigned long long int citatel[MAX_IT]={1, 1, 2, 17, 62, 1382, 21844, 929569,
                                        6404582, 443861162, 18888466084, 113927491862, 58870668456604}; // hodnoty citatela
    unsigned long long int menovatel[MAX_IT]={1, 3, 15, 315, 2835, 155925, 6081075, 638512875, 10854718875,
                                          1856156927625, 194896477400625, 49308808782358125, 3698160658676859375};// hodnoty menovatela

    for (int i=0;i<(int)n;i++)
    {
        tan=tan+(citatel[i]*koef/menovatel[i]); // vzorec na vypocet taylorovho radu
        koef=koef*x*x;
    }
    return tan;
}
double cfrac_tan(double x, unsigned int n) // funkcia na vypocet tan pomocou zretazenych zlomkov
{
    double moc=x*x;
    double vys=0;
    for (int i=n*2-1;i>=1;i=i-2)
    {
        vys=moc/(i-vys);
    }
    return vys/x;//treba este vydelit cislom x kedze na vrchu retazoveho zlomku je x a nie x^2
}

bool procesor(char *argv[],int argc)//funkcia na vypocet vysky objektu a vzdialenosti
{
    double vyska=1.5;//pokial nie vyska meriaceho pristroja inak zadana tak jej hodnota je 1.5
    double A,B;//uhol A a B
    char *p;
    double celkovaVyska=0;
    bool beta =false;
    if (strcmp(argv[1],"-c")==0)
    {
        vyska =strtod(argv[2],&p);// pokial stringova cast zo strod bude vacsia ako 0 tak zisti ze bol na mieste vysky zadany znak
        if (strlen(p)>0) errors(5);
        A = strtod(argv[4], NULL);
        if (A<=MIN||A>MAX_UHOL) errors(2); //pokial uhol A neni v povolenom rozsahu vypise error
        if (argc==6) //pokial je program spusteny so 6 argumentami, tak sa nacita aj Beta
        {
            B = strtod(argv[5], NULL);
            beta = true;
            if (B<=MIN||B>MAX_UHOL) errors(3); //pokial uhol B neni v povolenom rozsahu vypise error
        }
        if (argc<5) errors(1); //ked je argumentov menej ako 5 vypise error

    }
    else {
        A =strtod(argv[2],NULL); // nacita uhol A
        if (A<=MIN||A>MAX_UHOL) errors(2); //pokial uhol A neni v povolenom rozsahu vypise error
        if (argc==4) //pokial je program spusteny so 6 argumentami nacita uhol Beta
        {
            B =strtod(argv[3],NULL);
            beta = true;
            if (B<=MIN||B>MAX_UHOL) errors(3); //pokial uhol B neni v povolenom rozsahu vypise error
        }
    }



    if (vyska<=MIN||vyska>MAX_VYSKA) errors(4); //pokial vyska neni v povolenom rozsahu vypise error

    double distance=vyska/cfrac_tan(A,IT_COUNT);//vypocitanie vzdialenosti merajuceho od objektu
    printf("%.10e\n",distance);
    if (beta)
    {
        celkovaVyska=(distance*cfrac_tan(B,IT_COUNT))+vyska;//vypocet vysky objektu
        printf("%.10e\n",celkovaVyska);
    }

    return true;

}
int errors(int err) //funkcia na vypis errorov podla vlozeneho int argumentu
{
    switch (err)
    {
        case 0:
            fprintf(stderr,"Neplatny interval iteracii!!");
            exit(EXIT_FAILURE);
        case 1:
            fprintf(stderr,"Neplatny pocet argumentov!");
            exit(EXIT_FAILURE);
        case 2:
            fprintf(stderr,"Neplatny uhol A");
            exit(EXIT_FAILURE);
        case 3:
            fprintf(stderr,"Neplatny uhol B");
            exit(EXIT_FAILURE);
        case 4:
            fprintf(stderr,"Vyska meriacieho zariadenia je mimo povoleneho rozsahu");
            exit(EXIT_FAILURE);
        case 5:
            fprintf(stderr,"Neplatny znak na mieste vysky");
            exit(EXIT_FAILURE);
        case 6:
            fprintf(stderr,"Neplatne zadanie pre pomoc spustite program s argumentom --help");
            exit(EXIT_FAILURE);
    }
    return 0;

}