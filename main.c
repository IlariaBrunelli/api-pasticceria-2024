#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define B 1000
#define M 255

//VERSIONE FINALE

//definizione delle strutture

//Ingrediente
typedef struct Ingrediente Ingrediente;
struct Ingrediente {
    int quantita;
    void *magazzino;
    Ingrediente* next;
};

//Ricetta
typedef struct Ricetta Ricetta;
struct Ricetta {
    char nome[M];
    Ingrediente *ingredienti;
    int peso;
    Ricetta *next;
};

//Lotto
typedef struct Lotto Lotto;
struct Lotto {
    int data;
    int quantita;
    Lotto *next;
};

//Magazzino
typedef struct Magazzino Magazzino;
struct Magazzino {
    char nome[M];
    Lotto *headLotti; // Lista ordinata di lotti per ingrediente
    Magazzino *next;
};

//Ordine
typedef struct Ordine Ordine;
struct Ordine {
    int t;
    Ricetta* ricetta;  // Ricetta del prodotto
    int quantita;    // Quantità dell'ordine
    Ordine *next;
};

//CodaOrdine
typedef struct CodaOrdine CodaOrdine;
struct CodaOrdine {
    Ordine *front;  // Punta al primo elemento della coda
    Ordine *rear;   // Punta all'ultimo elemento della coda
};

//NodoAlbero
typedef struct NodoAlbero NodoAlbero;
struct NodoAlbero {
    Ordine *ordine;
    NodoAlbero *left;
    NodoAlbero *right;
};

//definisco le variabili
Ricetta *ricette[B];
Magazzino *magazzini[B];

CodaOrdine pronti;
CodaOrdine sospesi;

//ricette e magazzini con hash map
void inizializzaHashMap(){
    for(int i = 0; i < B; i++){
        ricette[i] = NULL;
        magazzini[i] = NULL;
    }
}

void inizializzaCode() {
    pronti.front = NULL;
    pronti.rear = NULL;

    sospesi.front = NULL;
    sospesi.rear = NULL;
}

int hash(const char *s, int m){
    int hash = 0;
    for(int i = 0; i < strlen(s); i++){
        hash = (hash * 31 + s[i]) % B;
    }
    return hash;
}


Ricetta *ricettaPresente(const char *nomeRicetta) {
    int b = hash(nomeRicetta, B);
    Ricetta *head = ricette[b];

    while(head) {
        if(strcmp(nomeRicetta, head->nome) == 0){
            return head;
        }

        head = head->next;
    }

    return NULL;

}

Magazzino *inserisciMagazzino(const char *ingrediente) {
    int b = hash(ingrediente, B);
    Magazzino *head = magazzini[b];

    if(head == NULL) {
        head = (Magazzino *) malloc(sizeof(Magazzino));
        head->headLotti = NULL;
        strcpy(head->nome, ingrediente);
        head->next = NULL;
        magazzini[b] = head;
        return head;
        
    } else {
        
        while(head) {
            if(strcmp(ingrediente, head->nome) == 0) {
                return head;
            }

            if(head->next == NULL) {
                head->next = (Magazzino *) malloc(sizeof(Magazzino));
                Magazzino *new = head->next;
                new->headLotti = NULL;
                new->next = NULL;
                strcpy(new->nome, ingrediente);
                return new;
            }

            head = head->next;
        }
    }

    return NULL;

}

void inserisciRicetta(const char *nome, Ingrediente *l, int peso) {
    int bucket = hash(nome, B);
    Ricetta *head = ricette[bucket];

    Ricetta *new = (Ricetta*) malloc(sizeof(Ricetta));
    new->ingredienti = l;
    new->next = head;
    new->peso = peso;
    ricette[bucket] = new;

    strcpy(new->nome, nome);
}

int ordiniProntiContains(const char *nome){
    Ordine* headPronti = pronti.front;
    while(headPronti) {
        if(strcmp(headPronti->ricetta->nome, nome) == 0) {
            return 1;
        }

        headPronti = headPronti->next;
    }

    return 0;
}

int ordiniSospesiContains(const char *nome) {
    Ordine* headSospesi = sospesi.front;
    while(headSospesi) {
        if(strcmp(headSospesi->ricetta->nome, nome) == 0) {
            return 1;
        }
        headSospesi = headSospesi->next;
    }

    return 0;
}

void rimuoviRicetta(const char* nome) {

    if(ordiniProntiContains(nome) || ordiniSospesiContains(nome)) {
        printf("ordini in sospeso\n");
    } else {
        int b = hash(nome, B);
        Ricetta *head = ricette[b];
        Ricetta *p = NULL;

        while(head) {
            if(strcmp(head->nome, nome) == 0) {
                break;
            }

            p = head;
            head = head->next;
        }

        if(head) {
            if(p) {
                p->next = head->next;
            } else {
                ricette[b] = head->next;
            }

            printf("rimossa\n");

            Ingrediente *lista = head->ingredienti;
            while(lista) {
                Ingrediente *next = lista->next;
                free(lista);
                lista = next;
            }

            free(head);
        } else {
            printf("non presente\n");
        }
    }

}


void rifornisci(const char *ingrediente, const int n, const int scadenza) {
    int b = hash(ingrediente, B);
    Magazzino *magazzino = magazzini[b];

    if(magazzino == NULL) {
        magazzino = (Magazzino *) malloc(sizeof(Magazzino));
        magazzino->headLotti = NULL;
        magazzino->next = NULL;
        strcpy(magazzino->nome, ingrediente);
        magazzini[b] = magazzino;
    } else {
        while(magazzino) {
            if(strcmp(magazzino->nome, ingrediente) == 0) {
                break;
            }
            magazzino = magazzino->next;
        }
    }

    Lotto *lotto = (Lotto *) malloc(sizeof(Lotto));
    lotto->data = scadenza;
    lotto->quantita = n;
    lotto->next = NULL;

    if(magazzino) {
        Lotto *lista = magazzino->headLotti;

        if(lista == NULL) {
            magazzino->headLotti = lotto;
        } else {
            if(lista->data >= lotto->data) {
                lotto->next = lista;
                magazzino->headLotti = lotto;
            } else {
                Lotto *prev = NULL;
                while(lista && lista->data < lotto->data) {
                    prev = lista;
                    lista = lista->next;
                }

                if(prev) {
                    Lotto *next = prev->next;
                    prev->next = lotto;
                    lotto->next = next;
                } else {
                    lista->next = lotto;
                }

            }
        }
    } else {
        magazzino = (Magazzino *) malloc(sizeof(Magazzino));
        magazzino->headLotti = lotto;
        magazzino->next = NULL;
        strcpy(magazzino->nome, ingrediente);

        Magazzino *t = magazzini[b];
        magazzini[b] = magazzino;
        magazzino->next = t;
    }

}

void inserisciOrdinePronto(Ordine *ordine) {
    if(pronti.front == NULL) {
        pronti.front = ordine;
        pronti.rear = ordine;
    } else {
        Ordine *r = pronti.front;
        Ordine *p = NULL;

        if(r->t > ordine->t) {
            ordine->next = r;
            pronti.front = ordine;

            return;
        }

        while(r && r->t < ordine->t) {
            p = r;
            r = r->next;
        }

        if(p) {
            if(p == pronti.rear) {
                pronti.rear = ordine;
            }
            p->next = ordine;
            ordine->next = r;
        } else {
            r->next = ordine;
        }
    }
}

void inserisciOrdinePronto2(Ordine *ordine, Ordine *ultimoPreparato) {

    Ordine *r = ultimoPreparato;
    Ordine *p = NULL;

    while(r && r->t < ordine->t) {
        p = r;
        r = r->next;
    }

    if(p) {
        if(p == pronti.rear) {
            pronti.rear = ordine;
        }
        p->next = ordine;
        ordine->next = r;
    } else {
        r->next = ordine;
    }

}

void inserisciOrdineSospeso(Ordine *ordine) {
    if(sospesi.front == NULL) {
        sospesi.front = ordine;
        sospesi.rear = ordine;
    } else {
        sospesi.rear->next = ordine;
        sospesi.rear = ordine;
    }
}


Lotto *liberaScaduti(Lotto *l, int t) {
    while(l && l->data <= t) {
        Lotto *next = l->next;
        free(l);
        l = next;
    }

    return l;
}

int verificaDisponibilitaLotti(Ordine *ordine, int t) {
    Ingrediente *li = ordine->ricetta->ingredienti;
    int quantita = ordine->quantita;

    while(li) {
        Magazzino *magazzino = li->magazzino;

        magazzino->headLotti = liberaScaduti(magazzino->headLotti, t);
        Lotto *lotto = magazzino->headLotti;

        if(lotto == NULL) {
            return 0;
        } else {
            int m = quantita * li->quantita;

            while(m > 0 && lotto != NULL) {
                m -= lotto->quantita;
                lotto = lotto->next;
            }

            if(m > 0) {
                return 0;
            }
        }
        li = li->next;
    }

    return 1;
}

void liberaLotti(Lotto *lotto) {
    while(lotto) {
        Lotto *next = lotto->next;
        free(lotto);
        lotto = next;
    }
}

void prelevaLotti(Ordine *ordine) {
    Ingrediente *li = ordine->ricetta->ingredienti;
    int quantita = ordine->quantita;

    while(li) {
        Magazzino *magazzino = li->magazzino;
        Lotto *lotto = magazzino->headLotti;

        int necessario = quantita * li->quantita;

        Lotto *prev = NULL;
        while(necessario > 0 && lotto != NULL) {
            if(lotto->quantita <= necessario) {
                necessario -= lotto->quantita;
                prev = lotto;
                lotto = lotto->next;
                free(prev);
            } else {
                lotto->quantita -= necessario;
                necessario = 0;
            }
        }

        magazzino->headLotti = lotto;

        li = li->next;
    }

    return;
}


void preparaOrdiniSospesi(int t) {
    Ordine *head = sospesi.front;
    Ordine *p = NULL;

    Ordine *u = NULL;

    while(head) {
        Ordine *next = head->next;
        if(verificaDisponibilitaLotti(head, t)) {
            prelevaLotti(head);

            head->next = NULL;

            if(p) {
                p->next = next;

                if(head == sospesi.rear) {
                    sospesi.rear = p;
                }
            } else {
                sospesi.front = next;

                if(head == sospesi.rear) {
                    sospesi.rear = NULL;
                }
            }

            if(u)
                inserisciOrdinePronto2(head, u);
            else
                inserisciOrdinePronto(head);

        } else {
            p = head;
        }

        head = next;
    }

}


void inserisciOrdineProntoZ(Ordine *ordine) {
    if(pronti.front == NULL) {
        pronti.front = ordine;
        pronti.rear = ordine;
    } else {
        pronti.rear->next = ordine;
        pronti.rear = ordine;
    }

    ordine->next = NULL;
}

int aBIG=0;

void call_function(const char *command, const int t) {
    if (strcmp(command, "aggiungi_ricetta") == 0) {
        char nomeRicetta[M];

        int a = scanf("%s", nomeRicetta);
aBIG=aBIG+a;

        if(ricettaPresente(nomeRicetta) != NULL){
            printf("ignorato\n");
            while (getchar() != '\n') {}
        } else {
            Ingrediente* lista = (Ingrediente*) malloc(sizeof(Ingrediente));;
            Ingrediente* head = lista;
            int p = 0;

            while(1) {
                char ingrediente[M];
                int quantita;

                int a2 =scanf("%s", ingrediente);
                int a3 =scanf("%d", &quantita);
aBIG=aBIG+a2+a3;

                lista->magazzino = inserisciMagazzino(ingrediente);

                lista->quantita = quantita;
                lista->next = NULL;

                p += quantita;

                char input = getchar();
                if(input == '\n'){
                    break;
                } else {
                    lista->next = (Ingrediente*) malloc(sizeof(Ingrediente));
                    lista = lista->next;
                }
            }

            inserisciRicetta(nomeRicetta, head, p);
            printf("aggiunta\n");
        }

    } else if (strcmp(command, "rimuovi_ricetta") == 0) {
            char nomeRicetta[M];

            int a4 =scanf("%s", nomeRicetta);
aBIG=aBIG+a4;

            rimuoviRicetta(nomeRicetta);

    } else if (strcmp(command, "rifornimento") == 0) {
        while (1) {
            char ingrediente[M];
            int quantita;
            int scadenza;

            int a5 =scanf("%s %d %d", ingrediente, &quantita, &scadenza);
aBIG=aBIG+a5;

            rifornisci(ingrediente, quantita, scadenza);

            char input = getchar();
            if(input == '\n'){
                    break;
            }
        }

        printf("rifornito\n");

        preparaOrdiniSospesi(t);


        //printLotti();
        //printOrdini();

    } else if (strcmp(command, "ordine") == 0) {
        char nomeRicetta[M];
        int quantita;

        int a6 = scanf("%s %d", nomeRicetta, &quantita);
aBIG=aBIG+a6;

        Ricetta *ricetta = NULL;
        ricetta = ricettaPresente(nomeRicetta);

        if(ricetta == NULL) {
            printf("rifiutato\n");
        } else {
            Ordine *ordine = (Ordine *) malloc(sizeof(Ordine));
            ordine->quantita = quantita;
            ordine->t = t;
            ordine->ricetta = ricetta;
            ordine->next = NULL;

            printf("accettato\n");

            if(verificaDisponibilitaLotti(ordine, t)) {
                prelevaLotti(ordine);
                inserisciOrdineProntoZ(ordine);
            } else {
                inserisciOrdineSospeso(ordine);
            }
        }

        //printLotti();
        //printOrdini();

    } else {
        printf("Unknown command: %s\n", command);
    }
}

void liberaCode(){
    Ordine* head = pronti.front;
    while(head) {
        Ordine* next = head->next;
        free(head);
        head = next;
    }

    head = sospesi.front;
    while(head) {
        Ordine* next = head->next;
        free(head);
        head = next;
    }
}

void liberaHashMaps() {
    for(int i = 0; i < B; i++) {
        Ricetta *headRicetta = ricette[i];
        while(headRicetta) {
            Ricetta *next = headRicetta->next;

            Ingrediente *headIngrediente = headRicetta->ingredienti;
            while(headIngrediente) {
                Ingrediente *next = headIngrediente->next;
                free(headIngrediente);
                headIngrediente = next;
            }

            free(headRicetta);
            headRicetta = next;
        }

        Magazzino *headMagazzino = magazzini[i];
        while(headMagazzino) {
            Magazzino *next = headMagazzino->next;

            Lotto *headLotto =headMagazzino->headLotti;
            while(headLotto) {
                Lotto *next = headLotto->next;
                free(headLotto);
                headLotto = next;
            }

            free(headMagazzino);
            headMagazzino = next;
        }
    }
}


void ritiro(int capienza) {
    Ordine *head = pronti.front;
    Ordine *r = head;
    Ordine *p = NULL;

    int i = 0;
    while(r && capienza - r->quantita * r->ricetta->peso >= 0) {
        capienza -= r->quantita * r->ricetta->peso;
        p = r;
        r = r->next;
        i++;
    }


    if(p) {
        p->next = NULL;
    }

    pronti.front = r;

    if(r == NULL) {
        pronti.rear = NULL;
    }


    if(i == 0) {
        printf("camioncino vuoto\n");
    }

    while (head) {
        Ordine *maxOrdine = head;
        Ordine *maxPrev = NULL;
        Ordine *current = head;
        Ordine *currentPrev = NULL;

        while (current) {
            if (current->quantita * current->ricetta->peso > maxOrdine->quantita * maxOrdine->ricetta->peso) {
                maxOrdine = current;
                maxPrev = currentPrev;
            }
            currentPrev = current;
            current = current->next;
        }

        printf("%d %s %d\n", maxOrdine->t, maxOrdine->ricetta->nome, maxOrdine->quantita);

        if (maxPrev) {
            maxPrev->next = maxOrdine->next;
        } else {
            head = maxOrdine->next;
        }

        free(maxOrdine);
    }
}



int main() {
    int period, capienza;

    if (scanf("%d %d", &period, &capienza) != 2) {
        fprintf(stderr, "Errore\n");
        return EXIT_FAILURE;
    }

    int t = 0;

    inizializzaHashMap();

    while (getchar() != '\n') {}

    char command[40];
    while (1) {

        if (t % period == 0 && t != 0) {
            ritiro(capienza);
        }

        int s = scanf("%s", command);
        if(s == EOF){
            break;
        }
        if(strcmp(command, "fine") == 0){
            break;
        }

        call_function(command, t);


        t++;
    }



    // liberaAlbero(radice);
    // liberaCoda(&coda);
    // liberaMagazzino(&magazzino);
    // liberaCodaPronti(&codaPronti);
    liberaCode();
    liberaHashMaps();

    return 0;
}

