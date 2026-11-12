# 🍰 Pasticceria industriale – Prova finale di Algoritmi e Strutture Dati

Progetto individuale per la **Prova finale di Algoritmi e Strutture Dati** (Algoritmi e Principi dell'Informatica), **Politecnico di Milano, A.A. 2023/24**.

Simulazione a tempo discreto di una pasticceria industriale, scritta in **C11 senza librerie esterne**.

> ⚡ **Lo scopo del progetto è l'efficienza.** Non basta produrre l'output corretto: un verificatore automatico esegue il programma su batterie di test di dimensione crescente e lo interrompe appena supera i limiti di **tempo** e di **memoria**. Ogni scelta progettuale è stata fatta per tenere bassi entrambi.

## Il problema

Il programma legge da `stdin` la configurazione del corriere (periodicità e capienza in grammi) e una sequenza di comandi, uno per riga. Dopo ogni comando trascorre un istante di tempo.

| Comando | Effetto | Risposta |
|---|---|---|
| `aggiungi_ricetta <nome> <ingrediente> <qtà> ...` | Aggiunge una ricetta al catalogo | `aggiunta` / `ignorato` |
| `rimuovi_ricetta <nome>` | Rimuove una ricetta se non ha ordini in sospeso | `rimossa` / `ordini in sospeso` / `non presente` |
| `rifornimento <ingrediente> <qtà> <scadenza> ...` | Aggiunge lotti al magazzino ed evade gli ordini in attesa possibili | `rifornito` |
| `ordine <ricetta> <numero>` | Registra un ordine e lo prepara subito, se possibile | `accettato` / `rifiutato` |

A ogni passaggio del corriere il programma stampa gli ordini caricati sul camioncino, oppure `camioncino vuoto`.

## Prestazioni

Il programma deve gestire sequenze molto lunghe di comandi entro limiti stretti di tempo macchina e di memoria, quindi le strutture dati sono state scelte in base al costo delle operazioni più frequenti.

### Tempo

| Operazione | Struttura usata | Costo |
|---|---|---|
| Cercare una ricetta (`aggiungi_ricetta`, `rimuovi_ricetta`, `ordine`) | Tabella hash | **O(1)** in media |
| Trovare un ingrediente in magazzino | Tabella hash + puntatore diretto salvato nella ricetta | **O(1)**, senza ricalcolare l'hash |
| Prelevare i lotti più vicini alla scadenza | Lista già ordinata per scadenza | si consuma dalla testa, **nessuna ricerca** |
| Eliminare i lotti scaduti | Rimozione pigra in testa alla lista | solo quando serve, **nessuna scansione periodica** |
| Verificare se un ordine è preparabile | Scorrimento dei soli lotti necessari | si ferma al **primo ingrediente insufficiente** |
| Mettere in coda un ordine pronto o in attesa | Code con puntatore alla fine | **O(1)** |
| Calcolare il peso di un ordine | Peso del dolce precalcolato nella ricetta | **O(1)**, una moltiplicazione |
| Scegliere gli ordini da caricare | Coda dei pronti già in ordine cronologico | **una sola scansione** dalla testa |

### Memoria

- **Nessuna stringa duplicata**: il nome di ogni ingrediente è salvato una sola volta nel magazzino; ricette e ordini contengono solo puntatori, non copie dei nomi.
- **Strutture minime**: un lotto contiene solo scadenza, quantità e puntatore al successivo; un ordine solo istante, quantità e puntatore alla ricetta.
- **Memoria restituita subito**: i lotti esauriti o scaduti e gli ordini spediti vengono liberati con `free` nel momento stesso in cui non servono più, quindi l'occupazione segue il numero di elementi *attivi*, non il numero di comandi letti.
- **Nessuna allocazione superflua**: gli ordini sbloccati da un rifornimento vengono spostati da una coda all'altra ricollegando i puntatori, senza copiarli.

## Ricettario

Le ricette sono in una **tabella hash con liste di trabocco** (1000 bucket, hash polinomiale in base 31 sul nome). Il nome di una ricetta viene cercato a ogni `aggiungi_ricetta`, `rimuovi_ricetta` e `ordine`, cioè nella maggior parte dei comandi: con la tabella hash la ricerca costa **O(1) in media** invece di O(n) su una lista.

Per risparmiare memoria e tempo:

- il **peso di un dolce** (somma dei grammi degli ingredienti) viene calcolato **una sola volta**, all'inserimento della ricetta, e salvato nella struttura: il corriere non lo ricalcola mai;
- gli ingredienti di una ricetta **non memorizzano il proprio nome**, ma solo la quantità e un **puntatore diretto** alla voce del magazzino. Ogni nome di ingrediente esiste quindi una sola volta in memoria, anche se compare in centinaia di ricette, e durante la preparazione non serve ricalcolare nessun hash.

## Magazzini

Anche il magazzino è una **tabella hash indicizzata per ingrediente**. Ogni ingrediente ha una **lista di lotti ordinata per scadenza crescente**: il nuovo lotto viene inserito direttamente nella posizione giusta, così il lotto più vicino alla scadenza è sempre in testa. Il prelievo, che è l'operazione più frequente, consuma la lista dalla testa senza alcuna ricerca e rispetta automaticamente la regola della specifica (si usano prima i lotti che scadono prima).

I **lotti scaduti vengono rimossi in modo pigro**: solo quando si controlla un ingrediente, liberando i lotti in testa con scadenza minore o uguale all'istante corrente (un lotto è scaduto *a partire* dall'istante indicato). Si evitano così scansioni periodiche dell'intero magazzino, e la memoria dei lotti scaduti o esauriti viene **restituita subito** con `free`, senza accumularsi.

## Preparazione

Un ordine va preparato **per intero oppure messo in attesa**. La preparazione è divisa in due fasi:

1. **verifica**: per ogni ingrediente si scorrono i lotti in testa finché la quantità richiesta è coperta, senza modificare nulla;
2. **prelievo**: solo se tutti gli ingredienti sono disponibili, si scalano le quantità e si liberano i lotti esauriti.

Questo evita di dover annullare un prelievo parziale, e la verifica si ferma appena trova un ingrediente insufficiente: il costo dipende solo dai lotti effettivamente necessari, non dalla dimensione del magazzino.

## Ordini

Gli ordini sono gestiti con **due code collegate**, e ogni ordine è una struttura minima (istante, quantità, puntatore alla ricetta): il nome della ricetta non viene mai copiato.

- **Ordini in attesa**: coda FIFO con puntatori a testa e coda. L'inserimento costa **O(1)** e, a ogni rifornimento, la coda viene scorsa dall'inizio, così gli ordini vengono smaltiti in ordine cronologico di arrivo; quelli che diventano preparabili vengono spostati nello stesso istante, senza allocare nuova memoria.
- **Ordini pronti**: coda mantenuta ordinata per istante di arrivo. Un ordine preparato subito viene aggiunto in fondo in **O(1)**, perché il tempo cresce sempre; solo un ordine sbloccato da un rifornimento, arrivato prima di altri già pronti, viene inserito nella posizione corretta.

`rimuovi_ricetta` controlla entrambe le code prima di eliminare una ricetta, così una ricetta con ordini non ancora spediti non viene mai cancellata.

## Corriere

Il passaggio del corriere viene gestito **prima** del comando dell'istante multiplo della periodicità, come richiesto. Grazie alla coda dei pronti già ordinata cronologicamente, la selezione degli ordini da caricare è un'unica scansione dalla testa, che si ferma al primo ordine che supera la capienza residua. Gli ordini scelti vengono staccati dalla coda in O(1), stampati in **ordine di peso decrescente** (a parità di peso, il confronto stretto mantiene l'ordine cronologico) e **liberati subito dopo la stampa**: in memoria restano solo gli ordini non ancora spediti.

## Compilazione ed esecuzione

Stesse opzioni del verificatore:

```bash
gcc -Wall -Werror -std=gnu11 -O2 -lm main.c -o main
./main < input.txt > output.txt
```

---

👩‍💻 Autrice: **Ilaria Brunelli** – Politecnico di Milano
