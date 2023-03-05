#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <math.h>

typedef struct {
    char nume[30];
    int nr_numere, id_Mapper;
}Fisier;

typedef struct {
    int nr_numere, nr_fisiere, nraux, *fisiere;
}Mapper;

typedef struct {
    int exponent, nr_baze, *baze;
}ListaExponent;  // folosit pentru agregarea tuturor puterilor
                 // perfecte pentru un anumit exponent

typedef struct {
    int factor, putere;
}TupluFactorPutere;  // folosit cand descompunem un numar in factori primi

typedef struct {
    int is_Mapper, is_Reducer, R, M;
    int nr_primes, *primes;  //  numerele prime < 46656 (Vezi README)
    int idM, nr_fisiere;
    char **fisiere;  // fisierele pentru toti mapperii
    int idR;

    // listele exponent-puteri pentru toti mapperii
    ListaExponent **exponenti_per_mapper;

    // listele exponent-puteri pentru toti reducerii
    // (agregate de la toti mapperii)
    ListaExponent *exponenti_per_reducer;

    pthread_barrier_t *barrier;
}My_arg;  // argument primit de thread-uri

// compara 2 fisiere dupa numarul de valori din ele
int cmpFisiere(const void *a, const void *b);

// functie ce descrie comportamentul threadurilor
void *thread_function(void *arg);

// numarul maxim de baze unice pentru un anumit exponent (baza ^ exponent < 2 ^ 31)
int nr_max_per_reduceri(int reducer); 

// verifica daca baza x se afla in lista pentru un anumit exponent
int cauta_putere_perfecta(int x, ListaExponent *lista);

// genereaza toate modurile de a scrie x ca si putere perfecta (cu exponentul <= R + 1)
void puteri_perfecte_all(int x, int *nr_rez, TupluFactorPutere *rez_list, int R, int nr_primes, int *primes);
