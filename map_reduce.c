#include "utils.h"

// no global variables :)

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Numar insuficient de argumente\n");
        return 0;
    }

    int M, R;

    M = atoi(argv[1]);
    R = atoi(argv[2]);

    if (!M || !R) {
        printf("I need some threads\n");
        return 0;
    }

    FILE *in = fopen(argv[3], "rt");

    int nr_fisiere, total_numere = 0;
    Mapper mappere[M];

    for (int i = 0; i < M; ++i) {
        mappere[i].nr_fisiere = mappere[i].nr_numere = mappere[i].nraux = 0;
    }

    fscanf(in, "%d", &nr_fisiere);
    Fisier fisiere[nr_fisiere];

    // parcurgem fisierele si retinem cate numere au fiecare
    // calculam si totalul de numere din toate fisierele
    for (int i = 0; i < nr_fisiere; ++i) {
        fscanf(in, "%s", fisiere[i].nume);
        FILE *fisier = fopen(fisiere[i].nume, "rt");
        fscanf(fisier, "%d", &fisiere[i].nr_numere);
        total_numere += fisiere[i].nr_numere;
        fclose(fisier);
    }
    
    if (M >= nr_fisiere) {
        // fiecare fisier va avea un mapper unic care se va ocupa doar de el
        for (int i = 0; i < nr_fisiere; ++i) {
            fisiere[i].id_Mapper = i;
            mappere[i].nr_fisiere = 1;
            mappere[i].nr_numere = fisiere[i].nr_numere;
        }
    } else {
        // numarul "ideal" de numere per mapper
        int numere_per_mapper = total_numere / M;
        if (M * numere_per_mapper < total_numere) {
            ++numere_per_mapper;
        }

        // sortam fisierele descrescator dupa numarul de valori de procesat
        qsort(fisiere, nr_fisiere, sizeof(Fisier), cmpFisiere);
        
        int id_mapper = 0, up = 1, i = 0;

        // vom parcurge toate fisierele si le vom distribui mapperelor 
        while (i < nr_fisiere) {
            // daca mapperul nu a depasit deja valoarea ideala de numere_per_mapper
            if (mappere[id_mapper].nr_numere < numere_per_mapper) {
                fisiere[i].id_Mapper = id_mapper;
                ++mappere[id_mapper].nr_fisiere;
                mappere[id_mapper].nr_numere += fisiere[i].nr_numere;
                ++i;
            }
            // luam mapperii crescator 0 1 ... M
            if(up) {
                ++id_mapper;
                if (id_mapper == M) {
                    // schimbam sensul
                    --id_mapper;
                    up = 0;
                }
            // luam mapperii descrescator M M-1 .. 0
            } else {
                --id_mapper;
                if (id_mapper == -1) {
                    //schimbam sensul
                    ++id_mapper;
                    up = 1;
                }
            }
        }
    }

    // ex: avem 4 mappere si 17 fisiere, o distribuire a fisierelor poate arata astfel:
    // 0 1 2 3 3 2 1 0 1 2 3 3 2 1 2 3 1
    // la sfarsit avem "3 1" in loc de "3 2" pt. ca mapperul 2 ar fi depasit numere_per_mapper

    for (int i = 0; i < M; ++i) {
        mappere[i].fisiere = malloc(mappere[i].nr_fisiere * sizeof(int));
    }
    
    for (int i = 0; i < nr_fisiere; ++i) {
        mappere[fisiere[i].id_Mapper].fisiere[mappere[fisiere[i].id_Mapper].nraux++] = i;
    }

    int k = 0, primes[5000], ciur[46657] = {0};

    // ciurul lui eratostene pentru numere < 46656 (vezi README)
    ciur[0] = ciur[1] = 1;
    for (int i = 2; i <= 216; ++i) {
        if (!ciur[i]) {
            for (int j = i * i; j <= 46656; j += i) {
                ciur[j] = 1;
            }
        }
    }
    // punem numerele prime intr-o lista
    primes[k++] = 2;
    for (int i = 3; i <= 46656; i += 2) {
        if (!ciur[i]) {
            primes[k++] = i;
        }
    }

    pthread_t *threads;
    pthread_barrier_t barrier;
    My_arg *thread_args;

    thread_args = (My_arg*)malloc((M + R) *sizeof(My_arg));
    threads = (pthread_t*)malloc((M + R) * sizeof(pthread_t));

    ListaExponent **exponenti_per_mapper, *exponenti_per_reducer;

    // alocam memorie pentru structurile ce tin listele exponent-putere
    // pentru fiecare mapper si pentru fiecare reducer

    exponenti_per_mapper = malloc(M * sizeof(ListaExponent *));
    for (int i = 0; i < M; ++i) {
        exponenti_per_mapper[i] = malloc((R + 2) * sizeof(ListaExponent));
        for (int j = 2; j <= R + 1; ++j) {
            exponenti_per_mapper[i][j].baze = malloc(nr_max_per_reduceri(j) * sizeof(int));
            exponenti_per_mapper[i][j].nr_baze = 0;
        }
    }
    exponenti_per_reducer = malloc((R + 2) * sizeof(ListaExponent));
    for (int i = 2; i <= R + 1; ++i) {
        exponenti_per_reducer[i].baze = malloc(nr_max_per_reduceri(i) * sizeof(int));
        exponenti_per_reducer[i].nr_baze = 0;
    }

    if (pthread_barrier_init(&barrier, NULL, M + R) != 0)
	{
		printf("Error can't initalize barrier");
		return 1;
	}
    
    for (int i = 0; i < M + R; ++i) {
        thread_args[i].barrier = (&barrier);
        thread_args[i].exponenti_per_mapper = exponenti_per_mapper;
        thread_args[i].exponenti_per_reducer = exponenti_per_reducer;
        thread_args[i].R = R;
        thread_args[i].M = M;
        if (i < M) {
            // avem un mapper
            thread_args[i].is_Mapper = 1;
            thread_args[i].is_Reducer = 0;
            thread_args[i].idM = i;
            thread_args[i].nr_fisiere = mappere[i].nr_fisiere;
            thread_args[i].nr_primes = k;
            thread_args[i].primes = primes;
            
            // punem numele fisierelor in argumentul mapperului
            thread_args[i].fisiere = malloc(mappere[i].nr_fisiere * sizeof(char *));
            for (int j = 0; j < mappere[i].nr_fisiere; ++j) {
                thread_args[i].fisiere[j] = malloc(strlen(fisiere[mappere[i].fisiere[j]].nume + 1));
                strcpy(thread_args[i].fisiere[j], fisiere[mappere[i].fisiere[j]].nume);
            }
        } else {
            // avem reducer
            thread_args[i].is_Reducer = 1;
            thread_args[i].is_Mapper = 0;
            thread_args[i].idR = i - M;
        }
		pthread_create(&threads[i], NULL, thread_function, &thread_args[i]);
	}
	// se asteapta thread-urile
	for (int i = 0; i < M + R; i++) {
		pthread_join(threads[i], NULL);
	}

    for (int i = 0; i < M; ++i) {
        free(mappere[i].fisiere);
    }

    pthread_barrier_destroy(&barrier); 
    
    // eliberam memoria folosita de structurile de date

    free(threads);
    free(thread_args);
    for (int i = 0; i < M; ++i) {
        for (int j = 2; j <= R + 1; ++j) {
            free(exponenti_per_mapper[i][j].baze);
        }
        free(exponenti_per_mapper[i]);
    }
    free(exponenti_per_mapper);
    for (int i = 2; i <= R + 1; ++i) {
        free(exponenti_per_reducer[i].baze);
    }
    free(exponenti_per_reducer);
    return 0;
}