#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <stddef.h>

#include "../Header/macro.h"
#include "../Header/Comunicazione.h"
#include "../Header/Trie.h"
#include "../Header/Lista.h"
#include "../Header/Matrice.h"
#include "../Header/FunzioniServer.h"
#include "../Header/Giocatore.h"

#define MAX_CLIENTS 32
#define MAX_LENGTH_USERNAME 10 // Numero massimo di lunghezza dell'username
#define NUM_THREADS 5          // Numero di thread da creare
#define BUFFER_SIZE 1024       // dimensione del buffer
#define MATRIX_SIZE 4
#define DIZIONARIO "../Dizionario.txt"

typedef struct
{
    char *matrix_file;
    float durata_partita;
    long seed;
    char *file_dizionario;
} Parametri;

void *scorer(void *arg);

// char* calcola_tempo_rimanente(time_t tempo_iniziale, int durata_partita);

int pausa_gioco = 0;     // Gioco
int durata_partita = 30; // La partita dura 5 minuti quindi 30s
int durata_pausa = 5;    // La pausa della partita dura 1 minuti
int punteggio = 0;
char *classifica; // Classifica non disponibile
int server_fd;
cella **matrice;
paroleTrovate *listaParoleTrovate = NULL;

Trie *Dizionario;
Parametri parametri;

// MUTEX
pthread_mutex_t pausa_gioco_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t matrix_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lista_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t scorer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t scorer_cond, game_cond, lista_cond;
pthread_mutex_t classifica_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t game_mutex;
pthread_t scorer_tid;
listaGiocatori lista; // Lista giocatori
Fifo *clients;        // Lista clienti
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
time_t tempo_iniziale;
Client *clients1;


#define TIMEOUT_MINUTES 2 // 2 minuti di inattività

// FUNZIONI PER IL 4 ADDENDUM
void *client_handler(void *arg)
{
    int client_index = *(int *)arg;
    free(arg);

    while (1)
    {
        pthread_mutex_lock(&clients_mutex);
        if (!clients1[client_index].active)
        {
            pthread_mutex_unlock(&clients_mutex);
            break;
        }

        time_t now = time(NULL);
        if (difftime(now, clients1[client_index].last_activity) > TIMEOUT_MINUTES * 60)
        {
            printf("Client %s inattivo, espulsione in corso...\n", clients1[client_index].username);
            clients1[client_index].active = 0; // Segna come non attivo
            close(clients1[client_index].fd);  // Chiudi il socket ??
            pthread_mutex_unlock(&clients_mutex);
            break;
        }
        pthread_mutex_unlock(&clients_mutex);
        sleep(1); // Controlla periodicamente
    }

    pthread_exit(NULL);
}

void update_client_activity(int client_socket)
{
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i)
    {
        if (clients1[i].fd == client_socket && clients1[i].active)
        {
            clients1[i].last_activity = time(NULL);
            printf("Aggiornata attività per %s.\n", clients1[i].username);
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// FUNZIONI
// Calcola tempo rimanente
char *calcola_tempo_rimanente(time_t tempo_iniziale, int durata_partita)
{
    time_t tempo_attuale = time(NULL);
    double tempo_trascorso = difftime(tempo_attuale, tempo_iniziale);
    double tempo_rimanente = durata_partita - tempo_trascorso;

    // Se il tempo rimanente è minore di 0 allora vuol dire che il gioco è finito
    if (tempo_rimanente < 0)
    {
        return "Il gioco è già terminato\n";
    }

    // Calcolo lunghezza del messaggio e alloco memoria
    int length = 64;
    char *messaggio = (char *)malloc(length + 1);

    // Verifica se l'allocazione è riuscita
    if (messaggio == NULL)
    {
        return "Errore di allocaione della memoria \n";
    }
    // Scrive il messaggio formattato nella memeoria allocata
    snprintf(messaggio, length + 1, "Il tempo rimanente è: %f secondi\n", tempo_rimanente);
    // return il messaggio
    return messaggio;
}
// TESTATA: FUNZIONA

// Funzione di invio classifica ai giocatori
void sendClassifica(listaGiocatori *lista, pthread_t tid, pthread_mutex_t lista_mutex, char *classifica)
{
    pthread_mutex_lock(&lista_mutex);
    // Inizializza un puntatore alla testa della lista
    giocatore *current = lista->head;
    while (current != NULL)
    {
        if (pthread_equal(current->tid, tid))
        {                                                                   // Controllo ID del thread del giocatore uguale all'ID del thread passato come parametro
            send_message(current->client_fd, MSG_PUNTI_FINALI, classifica); // Invia la classifica finale al fd del giocatore
        }
        current = current->next; // Passo al giocatore successivo
    }
    pthread_mutex_unlock(&lista_mutex);
}
// TESTATA : FUNZIONA

// Funzione per confrontare i punteggi dei giocatori -> qsort
int compare_score(const void *a, const void *b)
{
    giocatore *playerA = (giocatore *)a;              // Puntatore a convertito in un puntatore della struttura giocatore
    giocatore *playerB = (giocatore *)b;              // Puntatore b convertito in un puntatore della struttura giocatore
    return (playerB->punteggio - playerA->punteggio); // Confronto punteggi in ordine decrescente
}
// TESTATA: FUNZIONA

// HANDLER DEI SEGNALI
// Funzione di invio segnali a tutti i giocatori della lista
void invia_SIG(listaGiocatori *lista, int SIG, pthread_mutex_t lista_mutex)
{
    pthread_mutex_lock(&lista_mutex);
    giocatore *current = lista->head; // Inizializza un puntatore alla testa della lista dei giocatori
    while (current != NULL)
    {                                    // While finchè ci sono giocatori
        pthread_kill(current->tid, SIG); // Invia segnale SIG al thread current -> tid
        current = current->next;         // Passa al giocatore successivo
    }
    pthread_mutex_unlock(&lista_mutex);
}
// TESTATE: RUNTIME ERROR : Può essere per il SIG nei parametri?

// Funzione per cambiare stato del gioco
void alarm_handler(int sig)
{
    // Gestione del controllo dello stato del gioco
    if (pausa_gioco == 1)
    { // Se il gioco è in pausa
        pthread_mutex_lock(&pausa_gioco_mutex);
        pausa_gioco = 0; // Cambio lo stato del gioco per indicare il gioco in corso
        printf("Il gioco è in corso.\n");
        pthread_mutex_unlock(&pausa_gioco_mutex);
    }
    else
    { // Gestione scandenza tempo

        pthread_mutex_lock(&pausa_gioco_mutex);
        pausa_gioco = 1; // cambia lo stato del gioco
        printf("Il gioco è in pausa. \n");
        pthread_mutex_unlock(&pausa_gioco_mutex);

        // Invio del segnale a tutti i thread giocatori
        if (lista.count > 0)
        {
            // scorer = 1;
            invia_SIG(&lista, SIGUSR1, lista_mutex);                        // Invia segnale ai giocatori
            int retvalue = pthread_create(&scorer_tid, NULL, scorer, NULL); // Crea un nuovo thread per eseguire la funzione
            if (retvalue != 0)
            {
                perror("Errore nella pthread_create dello scorer");
            }
        }
    }
}
// TESTATE: RUNTIME ERROR

// Funzione per la chiusura del server
void sigint_handler(int sig)
{
    // Questa funzione chiude tutti quanti i client attivi (NON SOLO I GIOCATORI)
    pthread_mutex_lock(&clients_mutex);
    Client *current = clients->head;

    while (current != NULL)
    {
        Client *next = current->next;
        send_message(current->fd, MSG_FINE, "Il server è stato chiuso");
        free(current->username);
        free(current);
        current = next;
    }
    pthread_mutex_unlock(&clients_mutex);

    // Chiudi socket
    if (close(server_fd) == -1)
    {
        perror("Errore nella chiusura del socket del server");
    }

    printf("Chiusura del server \n");
    exit(EXIT_SUCCESS);
}
// TESTATE: RUNTIME ERROR

// Funzione per invio della classifica
void sig_classifica(int sig)
{
    pthread_mutex_lock(&lista_mutex);
    // Controllo se ci sono giocatori registrati
    if (lista.head == NULL)
    { // Se la testa è vuoto
        printf("Nessun giocatore registrato, classifica non disponibile \n");
        pthread_mutex_unlock(&lista_mutex);
        return;
    }
    // scorer = 0;
    sendClassifica(&lista, pthread_self(), lista_mutex, classifica);
    pthread_mutex_unlock(&lista_mutex);
}

// CARICO IL DIZIONARIO IN MEMORIA
void Load_Dictionary(Trie *Dictionary, char *path_to_dict)
{
    // APRO IL FILE TRAMITE IL PATH
    FILE *dict = fopen(path_to_dict, "r");
    // CREO UNA VARIABILE PER MEMORIZZARE LE PAROLE
    if (dict == NULL){
        fprintf(stderr, "Error: Could not open file %s/n", path_to_dict);
        return;
    }
    char word[256];
    // LEGGO TUTTO IL FILE
    while (fscanf(dict, "%s", word) != EOF)
    {
        // line[strlen(line)] = '\0';
       // printf("QUA? 2\n");
        //fflush(0);
        // STANDARDIZZO LE PAROLE DEL DIZIONARIO METTENDOLE IN UPPERCASE
        Caps_Lock(word);
        // printf("QUA? 3\n");
        // fflush(0);
        // // INSERISCO LA PAROLA NEL TRIE
        insert_Trie(Dizionario, word);
        // printf("QUA? 4\n");
        // fflush(0);
    }
    return;
}

// SOCKET
//  Funzione del thread
void *thread_func(void *args)
{

    // Francesco: Aggiungere ad una lista il client con il suo fd

    // Dichiara un puntatore per il valore di ritorno
    int client_sock = *(int *)args;

    Client *utente = malloc(sizeof(Client));
    utente->username = NULL;
    utente->fd = client_sock;
    utente->score = 0;
    utente->next = NULL;
    push(clients, utente);
    giocatore *giocatore = NULL;

    // MANCA UN PEZZO ?

    // Gestione dei comandi ricevuti dal client
    // MSG_MATRICE: invia la matrice e il tempo rimanente o il tempo di pausa
    // MSG_PAROLA: controllo punti della parola in base ai caratteri, se presente nella matrice, nel dizionario e accredita punti, se già trovata 0
    // MSG_REGISTRA_UTENTE: registra l'utente e controllo se già registrato
    // MSG_PUNTI_FINALI: calcolo i punti totali
    int retvalue;

    while (1)
    {
        message client_message = receive_message(client_sock);
        writef(retvalue, client_message.data);
        switch (client_message.type)
        {
        case MSG_MATRICE:
            if (pausa_gioco == 0)
            {

                // Gioco quindi invio la matrice attuale e il tempo di gioco rimanente
                stampaMatrice(matrice);
                invio_matrice(client_sock, matrice);
                char *temp = calcola_tempo_rimanente(tempo_iniziale, durata_partita);
                printf("Il tempo è %s", temp);
                send_message(client_sock, MSG_TEMPO_PARTITA, temp);
            }
            else
            {
                // Invio il tempo di pausa rimanente
                char *temp = calcola_tempo_rimanente(tempo_iniziale, durata_pausa);
                send_message(client_sock, MSG_TEMPO_ATTESA, temp);
            }
            break;

        case MSG_PAROLA:
            if (pausa_gioco == 0)
            {
                Caps_Lock(client_message.data);
                printf("La parola da cercare è %s\n", client_message.data);
                fflush(0);
                // Controllo se la parola è già stata trovata
                if (esiste_paroleTrovate(listaParoleTrovate, client_message.data))
                {
                    printf("ciao entro 1");
                    fflush(0);
                    send_message(client_sock, MSG_PUNTI_PAROLA, "0");
                    break;
                }

                // Controllo se parola è in matrice
                else if (!trovaParola(matrice, client_message.data))
                {
                    printf("ciao entro 2");
                    fflush(0);
                    send_message(client_sock, MSG_ERR, "Parola nella matrice non trovata");
                    break;
                }

                // Controllo se parola è nel dizionario
                else if (!search_Trie(client_message.data, Dizionario))
                {
                    printf("ciao entro 1");
                    fflush(0);
                    send_message(client_sock, MSG_ERR, "Parola nel dizionario non trovata");
                    break;
                }
                // Se i controlli hanno esito positivo, allora aggiungo parola alla lista delle parole trovate
                else
                {
                    printf("Aggiungo la parola alla lista delle parole trovate \n");
                    // Aggiungo la parola alla lista delle parole trovate
                    listaParoleTrovate = aggiungi_parolaTrovata(listaParoleTrovate, client_message.data); // DA SISTEMARE LA LISTA
                    int puntiparola = strlen(client_message.data);
                    printf("Punti parola \n");
                    // Se la parola contiene "Q" con "u" a seguito, sottraggo di uno i punti
                    if (strstr(client_message.data, "Qu"))
                    {
                        puntiparola--;
                        printf("Sottrai \n");
                    }
                    // Invio i punti della parola
                    char messaggiopuntiparola[50];
                    sprintf(messaggiopuntiparola, "%d", puntiparola);
                    send_message(client_sock, MSG_PUNTI_PAROLA, messaggiopuntiparola);
                    //sprintf("Punteggio inviato \n", messaggiopuntiparola);
                    printf("Punteggio inviato \n");
                    punteggio += puntiparola;
                }
            }
            else
            {
                // Invio il messaggio di errore
                send_message(client_sock, MSG_ERR, "Gioco in pausa!");
            }
            break;

        case MSG_REGISTRA_UTENTE:
            pthread_mutex_lock(&lista_mutex);
            registrazione_client(client_sock, client_message.data, &lista);
            pthread_mutex_unlock(&lista_mutex);

            break;

        case MSG_PUNTI_FINALI:
            if (pausa_gioco == 1 && classifica != NULL)
            {
                send_message(client_sock, MSG_PUNTI_FINALI, classifica);
                char *temp = calcola_tempo_rimanente(tempo_iniziale, durata_partita);
                send_message(client_sock, MSG_TEMPO_ATTESA, temp);
            }
            else
            {
                send_message(client_sock, MSG_ERR, "Classifica non disponibile");
            }
            break;

        case MSG_FINE:
            close(client_sock);
            pthread_exit(NULL);
            break;

        case MSG_CANCELLA_UTENTE:
            // Controllo se l'utente è loggato
            if (giocatore->username == NULL)
            {
                send_message(client_sock, MSG_ERR, "Utente non loggato");
                break;
            }
            printf("client_sock = %d, chiusura del client \n", client_sock);
            // Mi serve un elimina thread
            elimina_thread(clients, pthread_self(), &clients_mutex);
            elimina_giocatore(&lista, giocatore->username, lista_mutex);
            printf("giocatore [%s] disconesso \n", giocatore->username);
            close(client_sock);

            break;

            /* case MSG_LOGIN_UTENTE:
                 // Controllo se l'utente è loggato
                 if (giocatore->username != NULL)
                 {
                     send_message(client_sock, MSG_ERR, "Utente già loggato");
                     break;
                 }

                 listaGiocatori listatemp = RecuperaUtente (lista, message -> text);
                 if (listatemp == NULL) {
                         send_message(client_sock,MSG_ERR, "Errore, il giocatore non si è mai registrato. Registrazione utente");
                         break;
                     }
                     //Controllo se il giocatore è loggato in questo momento o meno
                     if (listatemp->loggato) {
                         send_message(client_sock, MSG_ERR, "Errore, un giocatore è già loggato con questo nome utente. Fare una nuova registrazione utente",);
                         break;
                     }
                 giocatore = listatemp;
                 giocatore -> username = client_sock;
                 send_message(client_sock, MSG_OK, "Utente loggato");
                 break;
            */
        default:
            send_message(client_sock, MSG_ERR, "Comando non valido");
            break;
        }
    }
}

void *scorer(void *arg)
{
    printf("Scorer in esecuzione\n");

    // Prendo il numero di giocatori registrati
    pthread_mutex_lock(&lista_mutex);
    int num_giocatori = lista.count;
    pthread_mutex_unlock(&lista_mutex);

    giocatore scorerVector[MAX_CLIENTS];
    giocatore *current = lista.head;

    pthread_mutex_lock(&lista_mutex);
    // Ciclo per raccogliere i risultati
    for (int i = 0; current != NULL && i < num_giocatori; i++)
    {
        scorerVector[i].username = strdup(current->username); // copia l'username
        scorerVector[i].punteggio = current->punteggio;       // Copia punteggio
        current = current->next;                              // Passa al prossimo giocatore
    }
    pthread_mutex_unlock(&lista_mutex);

    // Ordinamento dei risultati
    qsort(scorerVector, num_giocatori, sizeof(giocatore), compare_score);

    pthread_mutex_lock(&classifica_mutex);
    classifica = (char *)malloc(1024 * sizeof(char));
    classifica[0] = '\0';
    char msg[256];
    for (int i = 0; i < num_giocatori; i++)
    {
        sprintf(msg, "%s %d\n", scorerVector[i].username, scorerVector[i].punteggio);
        // strcat(classifica, msg, strlen(classifica) - 1 );
        strcat(classifica, msg);
        if (i < num_giocatori - 1)
        {
            strcat(classifica, ",");
        }
    }
    strcat(classifica, "\0");
    printf("Vincitore: %s con %d punti\n", scorerVector[0].username, scorerVector[0].punteggio);
    pthread_mutex_unlock(&classifica_mutex);

    for (int i = 0; i < num_giocatori; i++)
    {
        free(scorerVector[i].username);
    }
    printf("Classifica pronta. %d giocatori registrati.\n", num_giocatori);
    // Invio segnale a tutti i thread giocatori notificandoli che possono prelevare la classifica
    invia_SIG(&lista, SIGUSR2, lista_mutex); // Cambiato SIGINT in SIGUSR2

    // pthread_cond_broadcast(&classifica_mutex); // Notifico che la classifica è pronta
    // fai un for e invia ad ogni giocatore la classifica usando sendClassifica
    for (int i = 0; i < num_giocatori; i++)
    {
        sendClassifica(&lista, current->tid, lista_mutex, classifica);
    }
    return NULL;
}

// GESTISCE DEL GIOCO: perchè non funzionaa
void *game(void *arg)
{
    int round = 0;
    while (1)
    {
        if (lista.count == 0)
        {
            printf("Nessun giocatore registrato, attesa...\n");
            // ATTESA FINO A QUANDO NON SI REGISTRA UN NUOVO GIOCATORE

            while (lista.count == 0)
            {
                ;
            }
        }

        // Inizia la pausa
        printf("La partita è in pausa, inizierà tra: %d secondi\n", durata_pausa);
        sleep(durata_pausa);

        printf("Sono nel thread del gioco \n");
        // SE IL ROUND NON è STATO PREPARATO ALLORA LO PREPARA
        if (round == 0)
        {

            // Blocco per accedere alla matrice di gioco
            FILE *file = fopen("../Matrici.txt", "rb"); //
            Carica_MatricedaFile(file, matrice);        // Carica i dati della matrice dal file
            round = 1;
        }

        // INIZIA IL GIOCO
        time(&tempo_iniziale);
        // sleep(durata_partita);
        printf("-----------------------------------------------------------------------\n");
        printf("la partita è iniziata alle %ld con %d giocatori\n", tempo_iniziale, lista.count);

        // INVIO DELLA MATRICE E DEL TEMPO RIMANENTE AI GIOCATORI REGISTRATI E QUINDI PARTECIPANTI AL GIOCO
        pthread_mutex_lock(&lista_mutex);
        giocatore *current = lista.head;
        while (current != NULL)
        {
            invio_matrice(current->client_fd, matrice);
            char *temp = calcola_tempo_rimanente(tempo_iniziale, durata_partita);
            send_message(current->client_fd, MSG_TEMPO_PARTITA, temp);
            printf("Qua ci arrivo \n");
            current = current->next;
        }
        pthread_mutex_unlock(&lista_mutex);

        sleep(durata_partita);

        // partita terminata e quindi inviare punteggi classifiche ecc ecc ecec ec ec e c ec e c ece c ne ce c e ce

        // fai partire il thread dello scorer usando pthread_create

        // NOTIFICA CHE IL ROUND PREPARATO È STATO UTILIZZATO E QUINDI BISOGNA PREPARARNE UNO NUOVO
        round = 0;
    }
}

int main(int argc, char *argv[])
{
    Trie *trie = create_node();
    insert_Trie(trie, "CIAO");
    /*Dizionario = create_node();
    Load_Dictionary(Dizionario, DIZIONARIO);
    insert_Trie(Dizionario, "ciao");
    */
    printf("ciao %d\n", search_Trie("ciao", Dizionario));
    //Print_Trie();
    int server_sock;
    struct sockaddr_in server_addr;
    // char message [128];

    // Definizione dei segnali
    // signal(SIGUSR1, alarm_handler);

    if (argc < 3)
    {
        // Errore se numero dei parametri < 3
        printf("Errore: Numero errato di parametri passati");
        return 0;
    }
    // Creazione del socket
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    // Se la connessione fallisce, invio messaggio di errore
    if (server_sock < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    // Inizializzazione dell'indirizzo del server
    memset(&server_addr, 0, sizeof(server_addr));     // Azzerare la struttura
    server_addr.sin_family = AF_INET;                 // IPv4
    server_addr.sin_addr.s_addr = inet_addr(argv[1]); // Indirizzo server
    server_addr.sin_port = htons(atoi(argv[2]));      // Porta del server
    // creo ed inizializzo la lista utenti
    clients = create();

    signal(SIGINT, sigint_handler);
    // BIND() assegna l'indirizzo specificato nella struttura server_addr al socket server_sock
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        // Fallisce: messaggio di errore e chiusura socket
        perror("Bind fallita");
        close(server_sock);
        return 1;
    }

    // Se funziona il bind, il socket si mette in ascolto per poter accettare connessioni in entrata
    if (listen(server_sock, 5) < 0)
    {
        // Fallisce: messaggio di errore e chiusura socket
        perror("Listen fallita");
        close(server_sock);
        return 1;
    }

    printf("In attesa di connessioni...\n");
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    pthread_t gamer;
    int retvalue;
    matrice = generateMatrix();
    SYST(retvalue, pthread_create(&gamer, NULL, game, NULL), "nella creazione del thread di gioco");
    while (1)
    {
        // Accetta la connessione
        int client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_len);
        printf("Connessione accettata\n");
        // THREAD
        // Dichiara thread
        pthread_t thread_id;

        // Creazione del thread a cui passo il fd associato al socket del client
        if (pthread_create(&thread_id, NULL, thread_func, &client_sock) != 0)
        {
            // Se fallisce la creazione del thread, stampa un messaggio di errore e termina
            perror("Failed to create thread");
            return 1;
        }

        // Ciclo di comunicazione: aspetta che ogni thread termini e recupera il valore di ritorno
        // for (int i = 0; i < NUM_THREADS; i++)
        // { // DUBBIO
        //     pthread_join(thread_id, NULL);
        //     // Stampa il valore di ritorno dal thread
        //     // free(ret); // Libera la memoria allocata
        //     printf("thread ucciso\n");
        // }
    }
}