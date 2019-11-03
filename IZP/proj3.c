/**
 * Kostra programu pro 3. projekt IZP 2017/18
 *
 * Jednoducha shlukova analyza
 * Unweighted pair-group average
 * https://is.muni.cz/th/172767/fi_b/5739129/web/web/usrov.html
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h> // sqrtf
#include <limits.h> // INT_MAX
/*****************************************************************
 * Ladici makra. Vypnout jejich efekt lze definici makra
 * NDEBUG, napr.:
 *   a) pri prekladu argumentem prekladaci -DNDEBUG
 *   b) v souboru (na radek pred #include <assert.h>
 *      #define NDEBUG
 */
#ifdef NDEBUG

#else
#define debug(s)
#define dfmt(s, ...)
#define dint(i)
#define dfloat(f)

// vypise ladici retezec
#define debug(s) printf("- %s\n", s)

// vypise formatovany ladici vystup - pouziti podobne jako printf
#define dfmt(s, ...) printf(" - "__FILE__":%u: "s"\n",__LINE__,__VA_ARGS__)

// vypise ladici informaci o promenne - pouziti dint(identifikator_promenne)
#define dint(i) printf(" - " __FILE__ ":%u: " #i " = %d\n", __LINE__, i)

// vypise ladici informaci o promenne typu float - pouziti
// dfloat(identifikator_promenne)
#define dfloat(f) printf(" - " __FILE__ ":%u: " #f " = %g\n", __LINE__, f)

#endif

/*****************************************************************
 * Deklarace potrebnych datovych typu:
 *
 * TYTO DEKLARACE NEMENTE
 *
 *   struct obj_t - struktura objektu: identifikator a souradnice
 *   struct cluster_t - shluk objektu:
 *      pocet objektu ve shluku,
 *      kapacita shluku (pocet objektu, pro ktere je rezervovano
 *          misto v poli),
 *      ukazatel na pole shluku.
 */

struct obj_t {
    int id;
    float x;
    float y;
};

struct cluster_t {
    int size;
    int capacity;
    struct obj_t *obj;
};
void print_clusters(struct cluster_t *carr, int narr);

/*****************************************************************
 * Deklarace potrebnych funkci.
 *
 * PROTOTYPY FUNKCI NEMENTE
 *
 * IMPLEMENTUJTE POUZE FUNKCE NA MISTECH OZNACENYCH 'TODO'
 *
 */

void clear_cluster(struct cluster_t *c);
void errors(int err,int count,struct cluster_t *clusters)//errorove hlasenia
{
    switch (err)
    {
        case 0:
            fprintf(stderr,"neplatne suradnice objektu\n");
            break;
        case 1:
            fprintf(stderr,"nenaslo sa ID\n ");
            break;
        case 2:
            fprintf(stderr,"neplatny pocet argumentov\n");
            exit(EXIT_FAILURE);
        case 3:
            fprintf(stderr,"neplatny pocet zhlukov\n");
            exit(EXIT_FAILURE);
        case 4:
            fprintf(stderr,"zle zadany subor\n");
            exit(EXIT_FAILURE);
        case 5:
            fprintf(stderr,"neplatne zadanie!\n");
            break;
    }
	for(int i=0;i<count;i++)//uvolni jednotlive objekty v kazdom clustery
	  clear_cluster(&clusters[i]);
    	free(clusters);//a nakoniec uvolni samotny array
    exit(EXIT_FAILURE);


}

/*
 Inicializace shluku 'c'. Alokuje pamet pro cap objektu (kapacitu).
 Ukazatel NULL u pole objektu znamena kapacitu 0.
*/
void init_cluster(struct cluster_t *c, int cap)
{
    assert(c != NULL);
    assert(cap >= 0);
    c->size= 0;
    if (cap>0)
    {
        c->obj=malloc(cap* sizeof(struct obj_t));
        c->capacity=cap;
        return;
    }
    c->obj=NULL;
    c->capacity=0;
}

/*
 Odstraneni vsech objektu shluku a inicializace na prazdny shluk.
 */
void clear_cluster(struct cluster_t *c)
{
    free(c->obj);
}


/// Chunk of cluster objects. Value recommended for reallocation.
const int CLUSTER_CHUNK = 10;

/*
 Zmena kapacity shluku 'c' na kapacitu 'new_cap'.
 */
struct cluster_t *resize_cluster(struct cluster_t *c, int new_cap)
{
    // TUTO FUNKCI NEMENTE
    assert(c);
    assert(c->capacity >= 0);
    assert(new_cap >= 0);

    if (c->capacity >= new_cap)
        return c;

    size_t size = sizeof(struct obj_t) * new_cap;

    void *arr = realloc(c->obj, size);
    if (arr == NULL)
        return NULL;

    c->obj = (struct obj_t*)arr;
    c->capacity = new_cap;
    return c;
}

/*
 Prida objekt 'obj' na konec shluku 'c'. Rozsiri shluk, pokud se do nej objekt
 nevejde.
 */
void append_cluster(struct cluster_t *c, struct obj_t obj)
{
    if (c->capacity <= c->size) {
        if (resize_cluster(c, c->capacity + CLUSTER_CHUNK) == NULL) {
            return;
        }
    }
     c->obj[c->size++] = obj;
}

/*
 Seradi objekty ve shluku 'c' vzestupne podle jejich identifikacniho cisla.
 */
void sort_cluster(struct cluster_t *c);

/*
 Do shluku 'c1' prida objekty 'c2'. Shluk 'c1' bude v pripade nutnosti rozsiren.
 Objekty ve shluku 'c1' budou serazeny vzestupne podle identifikacniho cisla.
 Shluk 'c2' bude nezmenen.
 */
void merge_clusters(struct cluster_t *c1, struct cluster_t *c2)
{
    assert(c1 != NULL);
    assert(c2 != NULL);

    for (int i=0;i<c2->size;i++)
        append_cluster(c1,c2->obj[i]);
    sort_cluster(c1);
}

/**********************************************************************/
/* Prace s polem shluku */

/*
 Odstrani shluk z pole shluku 'carr'. Pole shluku obsahuje 'narr' polozek
 (shluku). Shluk pro odstraneni se nachazi na indexu 'idx'. Funkce vraci novy
 pocet shluku v poli.
*/
int remove_cluster(struct cluster_t *carr, int narr, int idx)
{
    assert(idx < narr);
    assert(narr > 0);
    clear_cluster(&carr[idx]); //uvolni cluster a zvysne posunie 
    for (int i=idx;i<narr-1;i++)
        carr[i]=carr[i+1];

    return narr-1;

}

/*
 Pocita Euklidovskou vzdalenost mezi dvema objekty.
 */
float obj_distance(struct obj_t *o1, struct obj_t *o2)
{
    assert(o1 != NULL);
    assert(o2 != NULL);
    float a=o1->x-o2->x;
    float b=o1->y-o2->y;
    return sqrtf(a*a+b*b);

}

/*
 Pocita vzdalenost dvou shluku.
*/
float cluster_distance(struct cluster_t *c1, struct cluster_t *c2)
{
    assert(c1 != NULL);
    assert(c1->size > 0);
    assert(c2 != NULL);
    assert(c2->size > 0);
    float distance=0;
    for(int i=0;i<c1->size;i++)//pocita priemer vzdielnosti medzi jednotlivymi objektami 2 zhlukov 
        for (int j=0;j<c2->size;j++)
            distance =distance+obj_distance(&c1->obj[i],&c2->obj[j]);

    return distance/(c1->size*c2->size);
}
float cluster_max_distance(struct cluster_t *c1, struct cluster_t *c2)
{
    assert(c1 != NULL);
    assert(c1->size > 0);
    assert(c2 != NULL);
    assert(c2->size > 0);
    float distance=0;
    for(int i=0;i<c1->size;i++)//pocita maximalnu vzdialenost medzi jednotlivymi objektami 2 zhlukov
        for (int j=0;j<c2->size;j++)
            if (distance<obj_distance(&c1->obj[i],&c2->obj[j])) distance = obj_distance(&c1->obj[i],&c2->obj[j]) ;

    return distance;
}
#define MAX_DISTANCE 2000;//maximalna mozna vzdialenost kedze x aj y moze byt najviac 1000

float cluster_min_distance(struct cluster_t *c1, struct cluster_t *c2)
{
    assert(c1 != NULL);
    assert(c1->size > 0);
    assert(c2 != NULL);
    assert(c2->size > 0);
    float distance=MAX_DISTANCE;
    for(int i=0;i<c1->size;i++)//pocita minimalnu vzdialenost medzi jednotlivymi objektami 2 zhlukov
        for (int j=0;j<c2->size;j++)
            if (distance>obj_distance(&c1->obj[i],&c2->obj[j])) distance = obj_distance(&c1->obj[i],&c2->obj[j]) ;

    return distance;
}


/*
 Funkce najde dva nejblizsi shluky. V poli shluku 'carr' o velikosti 'narr'
 hleda dva nejblizsi shluky. Nalezene shluky identifikuje jejich indexy v poli
 'carr'. Funkce nalezene shluky (indexy do pole 'carr') uklada do pameti na
 adresu 'c1' resp. 'c2'.
*/
int typ=0;

void find_neighbours(struct cluster_t *carr, int narr, int *c1, int *c2)
{
    assert(narr > 0);
    float distance=MAX_DISTANCE; 
    for (int i=0;i<narr;i++)//vypocita vzdialenost medzi 2 zhlukmi
        for (int j=i+1;j<narr;j++)

        switch (typ) {
            case 0:
                if (cluster_distance(&carr[i], &carr[j]) < distance) // pokial je nova vzdialenost mensia ako najmensia
                {                              //doteraz namerana tak sa distance zmeni a tiez aj c1 a c2
                    distance = cluster_distance(&carr[i], &carr[j]);
                    *c1 = carr[i].obj->id;
                    *c2 = carr[j].obj->id;
                }
                break;
            case 1:
                if (cluster_max_distance(&carr[i], &carr[j]) < distance) // pokial je nova vzdialenost mensia ako najmensia
                {                              //doteraz namerana tak sa distance zmeni a tiez aj c1 a c2
                    distance = cluster_max_distance(&carr[i], &carr[j]);
                    *c1 = carr[i].obj->id;
                    *c2 = carr[j].obj->id;
                }
                break;
            case 2:
                if (cluster_min_distance(&carr[i], &carr[j]) < distance) // pokial je nova vzdialenost mensia ako najmensia
                {                              //doteraz namerana tak sa distance zmeni a tiez aj c1 a c2
                    distance = cluster_min_distance(&carr[i], &carr[j]);
                    *c1 = carr[i].obj->id;
                    *c2 = carr[j].obj->id;
                }
                break;
        }

}

// pomocna funkce pro razeni shluku
static int obj_sort_compar(const void *a, const void *b)
{
    // TUTO FUNKCI NEMENTE
    const struct obj_t *o1 = (const struct obj_t *)a;
    const struct obj_t *o2 = (const struct obj_t *)b;
    if (o1->id < o2->id) return -1;
    if (o1->id > o2->id) return 1;
    return 0;
}

/*
 Razeni objektu ve shluku vzestupne podle jejich identifikatoru.
*/
void sort_cluster(struct cluster_t *c)
{
    // TUTO FUNKCI NEMENTE
    qsort(c->obj, c->size, sizeof(struct obj_t), &obj_sort_compar);
}

/*
 Tisk shluku 'c' na stdout.
*/
void print_cluster(struct cluster_t *c)
{
    // TUTO FUNKCI NEMENTE
    for (int i = 0; i < c->size; i++)
    {
        if (i) putchar(' ');
        printf("%d[%g,%g]", c->obj[i].id, c->obj[i].x, c->obj[i].y);
    }
    putchar('\n');
}

/*
 Ze souboru 'filename' nacte objekty. Pro kazdy objekt vytvori shluk a ulozi
 jej do pole shluku. Alokuje prostor pro pole vsech shluku a ukazatel na prvni
 polozku pole (ukalazatel na prvni shluk v alokovanem poli) ulozi do pameti,
 kam se odkazuje parametr 'arr'. Funkce vraci pocet nactenych objektu (shluku).
 V pripade nejake chyby uklada do pameti, kam se odkazuje 'arr', hodnotu NULL.
*/
#define MAX_COORD 1000
#define MIN_COORD 0
int load_clusters(char *filename, struct cluster_t **arr) {
    char var[10];
    int n;
    assert(arr != NULL);
    FILE *file;
    file = fopen(filename, "r"); // otvori subor s nazvom *filename
    if (file == NULL) {
        errors(4, 0, NULL);} //pokial je destinacia suboru neplatna vypise error}
        fgets(var, 9, file);
        sscanf(var, "count=%d", &n);//nacita pocet clusterov
        *arr = malloc(n * sizeof(struct cluster_t));//allocuje miesto pre array clusterov
        struct obj_t obj;
        for (int i = 0; i < n; i++) {
            init_cluster(&(*arr)[i], 1);
            fscanf(file, "%d ", &obj.id);//nacita jednotlive parametre
            fscanf(file, "%g ", &obj.x);
            if (obj.x > MAX_COORD || obj.x < MIN_COORD) {errors(0, n,*arr);}
            fscanf(file, "%g ", &obj.y);
            if (obj.y > MAX_COORD || obj.y < MIN_COORD) {errors(0, n,*arr);}
            append_cluster(&(*arr)[i], obj);//a nakoniec priradi objekt do clusteru
        }

        fclose(file);

        return n;

    }

/*
 Tisk pole shluku. Parametr 'carr' je ukazatel na prvni polozku (shluk).
 Tiskne se prvnich 'narr' shluku.
*/
    void print_clusters(struct cluster_t *carr, int narr) {
        printf("Clusters:\n");
        for (int i = 0; i < narr; i++) {
            printf("cluster %d: ", i);
            print_cluster(&carr[i]);
        }
    }
/*
 Najde objekt podla jeho ID a vrati idx clusteru v ktorom sa nachadza
*/
    int find_id(struct cluster_t *arr, int id, int count) {
        for (int i = 0; i < count; i++)
            for (int j = 0; j < arr[i].size; j++)
                if (arr[i].obj[j].id == id)
                    return i;
        errors(1, count, arr);
        return 0;
    }

int main(int argc, char *argv[]) {
        struct cluster_t *clusters;
	//printf("strcmp vracia:%d",strcmp(argv[3],"--max"));
        if (argc < 2 || argc > 4)
            errors(2, 0, NULL);        //pokial je viac alebo menej argumentov ako by malo vypise error
        int c1, c2, temp;
        int pocet;        //pokial uzivatel nezada pocet clusterov tak je vychodzia hodnota nastavena na 1
        if (argc < 3) pocet = 1; //pokial nie je zadane N tak je pocet nastaveny na vychodzu hodnotu 1
        else pocet = strtol(argv[2], NULL, 10);//nacita pocet clusterov zadany pouzivatelom
        if (pocet < 1) errors(3, 0, NULL);            //pokial je clusterov menej ako 1 vypise error
        int count = load_clusters(argv[1], &clusters);
        int fixed_count = count;            //ulozenie countu do inej premenej kedze count sa bude menit
        if (argc > 3) { 
            if (strcmp(argv[3],"--avg")==0 )
                for (int i = pocet; i < fixed_count; i++) {
                    find_neighbours(clusters, count, &c1, &c2);    //najde najblizsie dva zhluky
                    temp = find_id(clusters, c2,
                                   count);        //najde ID 2. z nich a ulozi do premennej kedze ho pouzijem 2 krat
                    merge_clusters(&clusters[find_id(clusters, c1, count)],
                                   &clusters[temp]);    //spoji tieto 2 najblizsie clustery
                    count = remove_cluster(clusters, count, temp);    //vymaze 2. z nich
                }
	    else if (strcmp(argv[3],"--max")==0) {
		 typ = 1;	 
		for (int i = pocet; i < fixed_count; i++) {
                    find_neighbours(clusters, count, &c1,
                                    &c2);    //najde najblizsie dva zhluky na principe najvzdialenejsieho suseda
                    temp = find_id(clusters, c2, count);
                    merge_clusters(&clusters[find_id(clusters, c1, count)], &clusters[temp]);
                    count = remove_cluster(clusters, count, temp);
                }
            } else if (strcmp(argv[3],"--min")==0 ) {
                typ = 2;
        		for (int i = pocet; i < fixed_count; i++) {
                    find_neighbours(clusters, count, &c1,
                                    &c2);//najde najblizsie dva zhluky na principe najlbizsieho suseda
                    temp = find_id(clusters, c2, count);
                    merge_clusters(&clusters[find_id(clusters, c1, count)], &clusters[temp]);
                    count = remove_cluster(clusters, count, temp);
                } 
			} else errors(5, count, clusters);
        } else
            for (int i = pocet; i < fixed_count; i++) {
                find_neighbours(clusters, count, &c1, &c2);
                temp = find_id(clusters, c2, count);
                merge_clusters(&clusters[find_id(clusters, c1, count)], &clusters[temp]);
                count = remove_cluster(clusters, count, temp);
            }

        print_clusters(clusters, count);
        for (int i = 0; i < count; i++)//uvolni jednotlive objekty v kazdom clustery
            clear_cluster(&clusters[i]);
        free(clusters);//a nakoniec uvolni samotny array

    }

