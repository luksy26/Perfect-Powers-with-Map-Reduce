# Perfect-Powers-with-Map-Reduce

Fisiere sursa:
	- map_reduce.c : citire din fisierul argument, distribuie fisiere threadurilor
		    mapper, alocare de memorie pentru structuri de date, construire
		    ciurul lui Eratostene, creare si join de threaduri (Mapperi si
		    Reduceri)
		    
	- utils.c : definitii pentru functiile folosite de threaduri si cele folosite
		    in tema1.c main()
		    
	- utils.h : antete pentru functiile din utils.c si definirea catorva structuri
		    cea mai relevanta este My_arg, care este data ca si argument la
		    creearea unui thread si contine toate informatiile despre threadul
		    curent sau pointeri catre zona de date comuna
		    
	- pthread_barrier_mac.h: pentru implementarea mecanismului de bariera in
				 threaduri
		    
Functia nr_max_per_reduceri:
	
	- functia e folosita pentru a aloca doar atat spatiu cat este suficient pentru
	a retine puterile perfecte pentru un anumit exponent

	- primeste un int (un exponent), si intoarce numarul maxim de numere > 0 care,
	ridicate la acest exponent, nu depasesc 2^31.
	- de exemplu, pentru exponentul 5, cel mai mare numar care ridicat la puterea
	5 si se incadreaza in conditie este 73 
	73 ^ 5 = 2 073 071 593, 74 ^ 5 = 2 219 006 624 prea mare
	- generalizat, este doar [2 ^ (31 / exponent]
	- pentru orice exponent mai mare de 19, avem nevoie de doar 2 numere
	3 ^ x > 2 ^ 31, cu x > 19 deci putem avea puteri perfecte doar cu baza 1 sau 2
	
In functia thread_function:

	- la linia 28 alocam spatiu doar pentru 7 numere
	- numarul de puteri perfecte pe care le poate avea un numar este dat de
	cati divizori comuni > 1 au exponentii din descompunerea in factori primi ai
	numarului
	
	ex: 20 736 = 2 ^ 8 * 3 ^ 4, 3 si 6 au divizori comuni {2,4}, deci numarul se
	poate scrie in 2 moduri: (2 ^ 4 * 3 ^ 2) ^ 2 si (2 ^ 2 * 3) ^ 4
	
	- in cel mai rau caz avem un singur factor prim, deci divizorii comuni > 1
	ai exponentilor vor fi chiar divizorii exponentului (unic).
	- consideram factorul prim 2 pentru a avea un exponent cat mai mare
	- cel mai mare exponent < 31 cu numar maxim de divizori este 24, cu 7
	divizori > 1 (https://en.wikipedia.org/wiki/Highly_composite_number)
	
	- asadar, un numar pozitiv x < 2 ^ 31 se poate scrie ca putere perfecta in
	maxim 7 moduri (2 ^ 24 este numarul)
	
In functia puteri_perfecte_all:

	- la linia 131 alocam spatiu doar pentru 9 numere
	- numarul maxim de factori primi pe care le poate avea un numar pozitiv
	x < 2 ^ 31 este 9
	- pentru a ajunge la aceasta valoare, luam cel mai rau caz in care exponentii
	factorilor primi este 1 si factorii primi sunt cele mai mici numere prime, in
	ordine
	- numarul este  223 092 870 = 2 * 3 * 5 * 7 * 11 * 13 * 17 * 19 * 23
	
Creare ciur in main:

	- vom gasi numerele prime mai mici decat 46656, deoarece 46656 ^ 2 > 2 ^ 31
	- daca o valoare x de intrare are un factor prim cu exponent mai mare decat 2,
	acesta este garantat sa fie in ciur
	- daca factorul prim nu se afla in ciur, acest lucru poate fi detectat daca
	parcurgem toate numerele din ciur ce pot fi divizori ai lui x (deci pana la
	sqrt(x)), si il reducem pe x impartind la factorii lui
	- daca la sfarsit x ramane > 1, acesta va fi factorul prim ce nu se afla in
	ciur, cu exponentul 1 (deci x nu se poate scrie ca putere perfecta)
	- stim ca daca avem un numar y, iar acesta nu se imparte la niciun numar prim
	<= cu sqrt(y), atunci y trebuie sa fie prim
	
Descrierea generala a pasilor algoritmului:

	- impartim fisierele in mod cat mai balansat Mapperilor
	- construim ciurul lui Eratostene
	- cream thread-urile Mapper si Reducer intr-o singura iteratie a thread-ului
	principal, cu argumentul corespunzator tipului de Thread
	- dam join la Threaduri
	- eliberam memoria
