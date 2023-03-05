#include "utils.h"

int cmpFisiere(const void *a, const void *b) {
    Fisier f1 = *(Fisier *)a;
    Fisier f2 = *(Fisier *)b;

    return f2.nr_numere - f1.nr_numere;
}

void *thread_function(void *arg) {
    My_arg my_arg = *(My_arg *)arg;
    int x, nr_rez;

    if(my_arg.is_Mapper) {
        // parcurgem fisierele pentru acest mapper
        for (int i = 0; i < my_arg.nr_fisiere; ++i) {
            FILE *fisier = fopen(my_arg.fisiere[i], "rt");
            int nr_numere;
            fscanf(fisier, "%d", &nr_numere);

            // citim numerele
            for (int j = 0; j < nr_numere; ++j) {
                fscanf(fisier, "%d", &x);
                nr_rez = 0;
                TupluFactorPutere *rez_list;

                // de ce doar 7? README
                rez_list = malloc(7 * sizeof(TupluFactorPutere));
                puteri_perfecte_all(x, &nr_rez, rez_list, my_arg.R, my_arg.nr_primes, my_arg.primes);

                // punem puterile perfecte in listele acestui mapper
                //          (fiecare exponent are o lista)
                for (int k = 0; k < nr_rez; ++k) {
                    int baza = rez_list[k].factor;
                    ListaExponent *lista = &(my_arg.exponenti_per_mapper[my_arg.idM][rez_list[k].putere]);
                    
                    // verificam daca puterea perfecta exista, nu dorim duplicate
                    if (!cauta_putere_perfecta(baza, lista)) {
                        lista->baze[lista->nr_baze] = baza;
                        ++(lista->nr_baze);
                    } 
                }
                free(rez_list);
            }
            fclose(fisier);
        }   
    }
    // asteptam sa termine toti mapperii inainte sa dam
    // merge la listele acestora prin reduceri
    pthread_barrier_wait(my_arg.barrier);

    if (my_arg.is_Reducer) {
        ListaExponent *lista_reducer = &(my_arg.exponenti_per_reducer[my_arg.idR + 2]);

        // parcurgem toti mapperii
        for (int i = 0; i < my_arg.M; ++i) {
            // luam lista mapperului pentru exponentul corespunzator reducerului curent
            ListaExponent *lista_mapper = &(my_arg.exponenti_per_mapper[i][my_arg.idR + 2]);

            for (int j = 0; j < lista_mapper->nr_baze; ++j) {
                // verificam daca puterea perfecta exista, nu dorim duplicate
                if (!cauta_putere_perfecta(lista_mapper->baze[j], lista_reducer)) {
                    lista_reducer->baze[lista_reducer->nr_baze] = lista_mapper->baze[j];
                    ++lista_reducer->nr_baze;
                }
            }
        }

    // cream fisierul de output si scriem in el
    char *filename = malloc(30);
    sprintf(filename,"out%d.txt", my_arg.idR + 2);
    FILE *out = fopen(filename, "wt");
    fprintf(out, "%d", my_arg.exponenti_per_reducer[my_arg.idR + 2].nr_baze);
    free(filename);
    fclose(out);
    }
    return NULL;
}

int nr_max_per_reduceri(int reduceri) { 
    // Vezi README
    switch(reduceri) {
        case 2: return 46340; break;
        case 3: return 1290; break;
        case 4: return 215; break;
        case 5: return 73; break;
        case 6: return 35; break;
        case 7: return 21; break;
        case 8: return 14; break;
        case 9: return 10; break;
        case 10: return 8; break;
        case 11: return 7; break;
        case 12: return 5; break;
        case 13: return 5; break;
        case 14: return 4; break;
        case 15: return 4; break;
        case 16: return 3; break;
        case 17: return 3; break;
        case 18: return 3; break;
        case 19: return 3; break;
        default: return 2;
    }
}

int cauta_putere_perfecta(int x, ListaExponent *lista) {
    for(int i = 0; i < lista->nr_baze; ++i) {
        if (lista->baze[i] == x) {
            return 1;
        }
    }
    return 0;
}

void puteri_perfecte_all(int x, int *nr_rez, TupluFactorPutere *rez_list, int R, int nr_primes, int *primes) {
    if (x <= 0) {
        // numere nule sau negative
        return;
    }
    if (x == 1) {
        *nr_rez = R;
        // luam toate puterile pana la R + 1
        for (int i = 0; i < R; ++i) {
            rez_list[i].factor = 1;
            rez_list[i].putere = i + 2;
        }
    return;
    }
    int min_exponent = 40, nr_factori = 0;

    // de ce doar 10? vezi README
    TupluFactorPutere *factori = malloc(9 * sizeof(TupluFactorPutere));

    // descompunem x in factori primi, parcurgand lista de nr prime
    for (int i = 0; primes[i] * primes[i] <= x && i < nr_primes; ++i) {
        if (x % primes[i] == 0) {
            factori[nr_factori].factor = primes[i];
            factori[nr_factori].putere = 0;
            while (x % primes[i] == 0) {
                x /= primes[i];
                ++factori[nr_factori].putere;
            }
            if (factori[nr_factori].putere < min_exponent) {
                min_exponent = factori[nr_factori].putere;
            }
            if (min_exponent == 1) { // avem un factor prim la puterea 1, deci nu putem face putere perfecta 
                return;
            }
            ++nr_factori;
        }
    }
    if (x > 1) {  // avem un factor prim la puterea 1, deci nu putem face putere perfecta 
        return;
    }
    if (R + 1 < min_exponent) {
        min_exponent = R + 1;
    }

    // exponentul maxim posibil pentru o putere perfecta a lui x
    // este exponentul minim din descompunerea in factori primi

    for (int d = 2; d <= min_exponent; ++d) {
        int i, baza = 1;

        // incercam sa constuim baza puterii perfecte
        for (i = 0; i < nr_factori; ++i) {
            if (factori[i].putere % d == 0) {
                baza *= pow(factori[i].factor, factori[i].putere / d);
            } else {
                // nu se poate forma putere perfecta cu exponentul d
                break;
            }
        }
        // toti exponentii factorilor primi s-au impartit la d
        if (i == nr_factori) {
            // obtinem o putere perfecta a lui x
            rez_list[*nr_rez].putere = d;
            rez_list[*nr_rez].factor = baza;
            ++(*nr_rez);
        }
    }
    free(factori);
}
