#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <stddef.h>

#include "../Header/macro.h"
#include "../Header/Comunicazione.h"
#include "../Header/Trie.h"
#include "../Header/Lista.h"
#include "../Header/Matrice.h"
#include "../Header/FunzioniServer.h"
#include "../Header/Giocatore.h"
#include "../Header/Bacheca.h"
#include "../Header/FileLog.h"

// DEFINIZIONE delle funzioni in FunzioniServer.h

#define MAX_CLIENTS 32
#define BUFFER_SIZE 1024                    // dimensione del buffer
#define MATRIX_SIZE 4
#define DIZIONARIO "../Dizionario.txt"
#define TIMEOUT_MINUTES 2                   // 2 minuti di inattività

int turno = 0;
int game_started = 0;
int pausa_gioco = 0;                        //Se 0 gioco, se 1 sono in pausa
int classifica_inviata = 0;                 //Classifica inviata o no
int durata_partita = 8;                     //La partita dura 5 minuti quindi 300s
int durata_pausa = 5;                       //La pausa della partita dura 1 minuti
int punteggio = 0;
char *classifica;                           //Classifica
int server_fd;

time_t tempo_iniziale;

cella **matrice;
paroleTrovate *listaParoleTrovate = NULL;
Trie *Dizionario;
Parametri parametri;
listaGiocatori lista;                       //Lista giocatori
Fifo *clients;                              //Lista clienti

/**********/
/**MUTEX**/
/********/
pthread_mutex_t pausa_gioco_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t matrix_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lista_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t scorer_mutex = PTHREAD_MUTEX_INITIALIZER;
//pthread_cond_t scorer_cond, game_cond, lista_cond;
pthread_mutex_t classifica_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t game_started_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t turno_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t scorer_tid;

//Definizioni funzioni
void* scorer(); 

/**********************/
/**FUNZIONI GENERALI**/
/********************/

//Funzione per calcolare il tempo rimanente
char *calcola_tempo_rimanente(time_t tempo_iniziale, int durata_partita){
    time_t tempo_attuale = time(NULL);                                          //Tempo attuale
    double tempo_trascorso = difftime(tempo_attuale, tempo_iniziale);           //Differenza tra il tempo attuale e il tempo iniziale
    double tempo_rimanente = durata_partita - (int)tempo_trascorso;             //Trovo tempo rimanente

    //Se il tempo rimanente è minore di 0 allora vuol dire che il gioco è finito
    if (tempo_rimanente < 0){
        return "Il gioco è già terminato\n";
    }

    int length = 64;                                                            //Calcolo lunghezza del messaggio e alloco memoria
    char *messaggio = (char *)malloc(length + 1);

    if (messaggio == NULL){                                                     //Verifica riuscita dell'allocazione
        return "Errore di allocaione della memoria \n";
    }
    //Scrive il messaggio formattato nella memeoria allocata
    snprintf(messaggio, length + 1, "Il tempo rimanente è: %d secondi\n", (int)tempo_rimanente); 
    return messaggio;
}

//Funzione per inviare la classifica
void sendClassifica(listaGiocatori *lista, pthread_t tid, char *classifica, time_t tempo_iniziale, int durata_pausa){

    //Controllo se la classifica è vuota, in tal caso errore
    if (classifica == NULL || strlen(classifica) == 0) {
        printf("[ERRORE] La classifica è NULL o vuota, non verrà inviata\n");
        return;
    }
    //Inizializzo il current alla testa dei client
    Client* current = clients->head;
    //Scansione 
    while (current != NULL) {
            if (current->isPlayer == 1){                                        //Controllo se client è attivo         
                printf("DEBUG: Invio classifica a %s: %s\n", current->username, classifica);
                send_message(current->fd, MSG_PUNTI_FINALI, classifica);        //Invio la classifica
                printf("DEBUG: Classifica inviata a %s: %s\n", current->username, classifica);
            } else {
                printf("ERRORE: Socket di %s non valido\n", current->username);
            }
            current = current->next;
        }
    //pthread_mutex_unlock(&clients_mutex); -> non le ho aperte
    //pthread_mutex_unlock(&lista_mutex);
}

// Funzione per confrontare i punteggi dei giocatori -> qsort
int compare_score(const void *a, const void *b){
    giocatore *playerA = *(giocatore **)a;                                      //Puntatore a convertito in un puntatore della struttura giocatore
    giocatore *playerB = *(giocatore **)b;                                      //Puntatore b convertito in un puntatore della struttura giocatore
    return playerB->punteggio - playerA->punteggio;                             //Confronto punteggi in ordine decrescente
}

// Funzione per gestire i punteggio, cercando il giocatore nella lista col tid
int prendi_punteggi(listaGiocatori* lista, pthread_t tid){
    pthread_mutex_lock(&lista_mutex);                                           //Mutex per avere l'accesso esclusivo alla lista
    
    //Inizializzo current alla testa della lista
    giocatore *current = lista->head;
    //Scansione lista
    while(current != NULL){
        if (pthread_equal(current -> tid, tid)){                                //Controllo se tid corrisponde
            int punteggio =  current->punteggio;                                //Ottengo punteggio
            pthread_mutex_unlock(&lista_mutex);                                 //Rilascio mutex
            return punteggio;
        }
        current = current->next;                                                //Puntatore al prossimo giocatore
    }
    pthread_mutex_unlock(&lista_mutex);
    return -1;                                                                  //Return -1 quando giocatore non trovato
}

//Funzione per resettare i punteggi dei giocatori
void reset_punteggi() {
    pthread_mutex_lock(&lista_mutex);                              //Mutex per accesso esclusivo della lista
    //Inizializzo current alla testa della lista
    giocatore *current = lista.head;
    //Scansione
    while (current != NULL) {
        current->punteggio = 0;                                    //Punteggio = 0
        current = current->next;                                   //Puntatore al prossimo giocatore
    }
    pthread_mutex_unlock(&lista_mutex);                            //Rilascio mutex
}

//Funzione per caricare il dizionario in memoria
void Load_Dictionary(Trie *Dictionary, char *path_to_dict){    
    FILE *dict = fopen(path_to_dict, "r");                                      //Apertura file in modalità lettura "r"

    if (dict == NULL){                                                          //Controllo se apertura del file non ha avuto successo, in tal caso errore
        fprintf(stderr, "[Errore] Impossibile aprire il file %s/n", path_to_dict);
        return;
    }

    char word[256];                                                             //Dichiaro array per memorizzare le parole 
    
    //Scansione del file
    while (fscanf(dict, "%s", word) != EOF){                                    
        Caps_Lock(word);                                                        //Converte in maiuscolo la parola
        insert_Trie(Dizionario, word);                                          //Inserisce la parola nel Trie
    }
    return;
}

//Funzione per trovare giocatore nella lista
giocatore* trova_giocatore(listaGiocatori* lista, pthread_t tid){
    
    //Inizializzo corrente alla testa della lista
    giocatore* corrente = lista->head;

    //Scansione
    while (corrente != NULL) {
        if (pthread_equal(corrente->tid, tid)){                                //Controllo se tid corrisponde
            return corrente;                                                   //Giocatore trovato, restituisco il puntatore
        }
        corrente = corrente->next;                                             //Puntatore al prossimo giocatore
    }

    return NULL;                                                               //Giocatore non trovato
}

/************************/
/**HANDLER DEI SEGNALI**/
/**********************/

// Funzione di invio segnali a tutti i giocatori della lista
void invia_SIG(listaGiocatori *lista, int SIG, pthread_mutex_t lista_mutex){
    pthread_mutex_lock(&clients_mutex);                                        //Mutex per accesso esclusivo dei clients
    printf("Invio segnale %d a tutti i giocatori \n", SIG);

    //Inizializzo current alla testa dei clients
    Client* current = clients->head;

    //Scansione 
    while (current != NULL){   
        //if(current->isPlayer== 1){                                 
            printf("Invio segnale %d al giocatore con tid %ld\n", SIG, current->thread_id);
            pthread_kill(current->thread_id, SIG);                              //Invia segnale SIG al thread current -> tid
        //} else {
            printf ("DEBUG: Il client %s è già disconesso, segnale non inviato\n", current->username);
        //}
            current = current->next;                                            //Passa al giocatore successivo
    }
    pthread_mutex_unlock(&clients_mutex);                                       //Rilascio mutex
}

// Funzione per la gestione del segnale relativo all'invio classifica
void sigusr2_classifica_handler(int sig){

    if (classifica_inviata){                                                    //Controllo se classifica già stata inviata
        printf("DEBUG: Il gestore SIGUSR2 è già stato chiamato, salto.\n");
        return;
    }
    classifica_inviata = 1;

    printf("Gestore del segnale SIGUSR2 chiamato \n");

    pthread_mutex_lock(&lista_mutex);                                           //Mutex per accesso esclusivo alla lista
    
    if (lista.head == NULL){                                                    //Controllo se testa della lista vuota
        printf("Nessun giocatore registrato, classifica non disponibile \n");
        pthread_mutex_unlock(&lista_mutex);                                     //Rilascio mutex
        return;
    }
    pthread_mutex_unlock(&lista_mutex);                                         //Rilascio mutex

    printf("Segnale SIGUSR2 ricevuto, la classifica è pronta per essere inviata ai giocatori\n");

    // Genera la classifica
    pthread_create(&scorer_tid, NULL, scorer, NULL);
    pthread_join(scorer_tid, NULL);

    if (classifica == NULL) {                                                   //Controlliamo se la classifica è NULL
        printf("ERRORE: La classifica è NULL dopo scorer(), impossibile inviarla\n");
        return;
    }

    // Invia la classifica ai giocatori
    sendClassifica(&lista, pthread_self(), classifica, tempo_iniziale, durata_pausa);

    pthread_mutex_lock(&pausa_gioco_mutex);
    pausa_gioco = 1;
    pthread_mutex_unlock(&pausa_gioco_mutex);

    alarm(durata_pausa);                                                        //Alarm per la durata della pausa
}

//Funzione per gestire il segnale CTRL+C da parte del server
void sigint_handler(int sig){
    
    pthread_mutex_lock(&clients_mutex);                                             //Mutex per accesso esclusivo dei clients

    //Inizializzo il current alla testa dei client
    Client *current = clients->head;

    //Scansione
    while (current != NULL){ 
        Client *next = current->next;                                              //Puntatore prossimo al client successivo
        send_message(current->fd, MSG_SERVER_SHUTDOWN, "Il server è stato chiuso");//Manda messaggio di chiusura
        free(current->username);                                                   //Libera memoria 
        free(current);
        current = next;
    }
    pthread_mutex_unlock(&clients_mutex);                                          //Rilascio mutex

    //CHIUSURA DEL SOCKET
    if (close(server_fd) == -1){                                                   //Se chiusura fallisce, errore
        perror("Errore nella chiusura del socket del server");
    }
    printf("Chiusura del server \n");                                              //Stampa messaggio di chiusura e termina
    exit(EXIT_SUCCESS);
}

//Funzione per gestire il segnale di ALARM
void alarm_handler(int sig){
    
    static int sigusr2_inviato = 0;                                                 //Flag per segnale inviato

    //Gestione degli stati di gioco
    switch (pausa_gioco){
        //Caso 1: sono in pausa
        case 1:
            alarm(durata_partita);                                                 //Alarm per durata partita
            pausa_gioco = 0;                                                       //Cambio stato della pausa
            printf("[ALARM]: Allarme partita mandato\nIl gioco è in corso\n");
            sigusr2_inviato = 0;                                                   //Reset variabile
            return;

        //Caso 0: in gioco
        case 0:
            turno = 0;
            game_started = 0;
            pausa_gioco = 1;                                                       //Cambio stato della pausa
            printf("La partita è finita, inizierà tra: %d secondi\n", durata_pausa);

            if (!sigusr2_inviato){                                                 //Controllo se segnale non inviato
                //Controllo se ci sono giocatori registrati
                if (lista.count > 0) {
                    invia_SIG(&lista, SIGUSR2, lista_mutex);                       //Invia segnale ai giocatori
                    printf("Invio segnale SIGUSR2\n");
                    sigusr2_inviato = 1;                                           //Cambio stato del sigusr2_inviato, in modo da non avere multipli 

                } else {
                    printf("DEBUG: Nessun giocatore registrato, SIGUSR2 non inviato.\n");
                }
            } else {
                printf("DEBUG: SIGUSR2 già inviato\n");
            }
                alarm(durata_pausa);                                               //Allarme durata pausa
                printf("Il gioco è terminato\n");
        break;
    }
}

/*****************/
/*FUNZIONE THREAD*/
/*****************/
void *thread_func(void *args){
    int client_sock = *(int *)args;
    
    //Alloca memoria per il client
    Client *utente = malloc(sizeof(Client));
    utente->username = NULL;
    utente->fd = client_sock;
    utente->score = 0;
    utente->next = NULL;
    utente->thread_id = pthread_self();
    push(clients, utente);                                      //Aggiunge il client alla lista 

    pthread_mutex_lock(&lista_mutex);                           //Mutex per accesso esclusivo della lista
    utente->last_activity = time(NULL);                         //Aggiorna il timestamp dell'attività
    pthread_mutex_unlock(&lista_mutex);                         //Rilascio mutex

    int retvalue;

    //Ciclo del thread
    while (1){
        message client_message = receive_message(client_sock);  //Ricezione del messsaggio
        pthread_mutex_lock(&lista_mutex);                       //Mutex per accesso esclusivo della lista
        utente->last_activity = time(NULL);                     //Aggiorna il timestamp dell'attività
        pthread_mutex_unlock(&lista_mutex);                     //Rilascio mutex

        writef(retvalue, client_message.data);                  //Per visualizzare il messaggio ricevuto

        //Gestione del messaggio in base al tipo
        switch (client_message.type){

            case MSG_MATRICE:                                   //Richiesta matrice di gioco
            if (utente->isPlayer == 0){                         //Controllo se client non è attivo, in tal caso errore
                send_message(client_sock, MSG_ERR, "Devi essere loggato per richiedere la matrice di gioco");
                break;
            }
            if (pausa_gioco == 0){                              //Se pausa = 0, vuol dire che gioco 
                stampaMatrice(matrice);                         //Stampo e invio della matrice
                invio_matrice(client_sock, matrice); 
                char *temp = calcola_tempo_rimanente(tempo_iniziale, durata_partita);   //Calcola tempo rimanente di gioco
                printf("Il tempo è %s", temp);
                send_message(client_sock, MSG_TEMPO_PARTITA, temp);                     //Manda un messaggio con tempo rimanente
            } else {
                char *temp = calcola_tempo_rimanente(tempo_iniziale, durata_pausa);     //Calcola tempo rimanente delle pausa
                send_message(client_sock, MSG_TEMPO_ATTESA, temp);                      //Manda un messaggio con tempo di attesa
            }
            break;

            case MSG_PAROLA:                                     //Invio di parola da cercare
            if (utente->isPlayer == 0){                         //Controllo se client non è attivo, in tal caso errore
                send_message(client_sock, MSG_ERR, "Devi essere loggato per indicare la parola");
                break;
            }
            if (pausa_gioco == 0){                                              //Gioco in corso
                client_message.data[strcspn(client_message.data, "\n")] = '\0'; //Rimuove eventuali \n
                client_message.data[strcspn(client_message.data, "\r")] = '\0'; //Rimuove eventuali \r

                Caps_Lock(client_message.data);                                 //Converto il messaggio in maiuscolo
                printf("La parola da cercare è %s\n", client_message.data);
                fflush(0);

                //Controllo se la parola è già stata trovata
                if (esiste_paroleTrovate(utente->paroleTrovate, client_message.data)){
                    send_message(client_sock, MSG_PUNTI_PAROLA, "0");
                    break;
                }
                // Controllo se parola è in matrice
                else if (!trovaParola(matrice, client_message.data)){
                    printf("Debug: Tentativo di trovare %s nella matrice.\n", client_message.data);
                    printf("Matrice attuale:\n");
                    stampaMatrice(matrice);                                       
                    printf("Debug: La parola non è stata trovata. \n");
                    send_message(client_sock, MSG_ERR, "Parola nella matrice non trovata");
                    break;
                }
                // Controllo se parola è nel dizionario
                else if (search_Trie(client_message.data, Dizionario) == -1){
                    send_message(client_sock, MSG_ERR, "Parola nel dizionario non trovata");
                    break;
                    }
                // Se i controlli hanno esito positivo, allora aggiungo parola alla lista delle parole trovate
                    else{
                        printf("Aggiungo la parola alla lista delle parole trovate \n");
                        utente->paroleTrovate = aggiungi_parolaTrovata(listaParoleTrovate, client_message.data); 
                        // Recupero il punteggio del giocatore attuale
                        int punteggio_corrente = prendi_punteggi(&lista, pthread_self());
                            if(punteggio_corrente == -1){
                                printf("Errore: giocatore non trovato\n");
                                send_message(client_sock, MSG_ERR, "Errore nel recupero del punteggio");
                                break;
                            }
                        
                        int punti_parola = strlen(client_message.data);
                        printf("Punti parola \n");
                            // Se la parola contiene "Q" con "u" a seguito, sottraggo di uno i punti
                            if (strstr(client_message.data, "QU")){
                                printf("Sottrai \n");
                                punti_parola = punti_parola - 1;
                            }
                        pthread_mutex_lock(&lista_mutex);

                        giocatore* player = trova_giocatore(&lista, pthread_self());
                            if (player != NULL){
                                player -> punteggio += punti_parola;
                            }
                        pthread_mutex_unlock(&lista_mutex);

                        //file_log(utente->username, client_message.data);

                        char messaggiopuntiparola[90];
                        printf("Punti inviati %d, \n", punti_parola);
                        sprintf(messaggiopuntiparola, "Con questa parola hai ottenuto %d punti", punti_parola);
                        send_message(client_sock, MSG_PUNTI_PAROLA, messaggiopuntiparola);
                        printf("Punteggio inviato: %d\n", punti_parola);
                        printf("Il punteggio attuale è %d \n", punteggio_corrente); // Nel server visualizzo i punti totali
                        fflush(0);
                    }
            } else {
            // Invio il messaggio di errore
                send_message(client_sock, MSG_ERR, "Gioco in pausa!");
            }
            break;

            case MSG_REGISTRA_UTENTE:                                               //Richiesta di registrazione
            pthread_mutex_lock(&lista_mutex);

            //Controllo se la registrazione ha avuto successo
            if (registrazione_client(client_sock, client_message.data, &lista) == -1){     //Registrazione di un nuovo client
                pthread_mutex_unlock(&lista_mutex);
                break;
            }

            //Inizializzo un newplayer alla testa della lista
            giocatore *newPlayer = lista.head;
            int aggiunto = 0; 
                //Scansione
                while (newPlayer != NULL) {
                    if (strcmp(newPlayer->username, client_message.data) == 0){     //Controllo se username corrisponde
                        newPlayer->active = 1;
                        aggiunto = 1;                                               //Attivo il giocatore
                        break;
                    }
                    newPlayer = newPlayer->next;
                }
            pthread_mutex_unlock(&lista_mutex);
            
            if (aggiunto){
                pthread_mutex_lock(&clients_mutex);

                //Inizializzo current alla testa dei client
                Client* current = clients->head;
            
                    while(current != NULL){
                        if(pthread_equal(current->thread_id, pthread_self())){          //Controllo se corrisponde il tid
                            current->isPlayer = 1;                                      //Attivo il client
                            current->username = strdup(client_message.data);            //Duplico la stringa dell'username    
                            break;
                        }
                        current = current->next;
                    }
                pthread_mutex_unlock(&clients_mutex);
                send_message(client_sock, MSG_OK, "Registrazione avvenuta con successo e login automatico.");
                printf("[REGISTRAZIONE] avvenuta");
            }
            break;
            

            case MSG_PUNTI_FINALI:                                                      //Richiesta di invio, classifica
            pthread_mutex_lock(&pausa_gioco_mutex);
            printf("[CLASSIFICA] RICHIESTA CLASSIFICA RICEVUTA, pausa gioco %d, classifica:%s\n", pausa_gioco, classifica ? classifica: "NULL");
                if (pausa_gioco == 1 && classifica != NULL){                            //Se gioco in pausa e classifica non NULL, invio 
                    send_message(client_sock, MSG_PUNTI_FINALI, classifica);
                    char *temp = calcola_tempo_rimanente(tempo_iniziale, durata_pausa);
                    send_message(client_sock, MSG_TEMPO_ATTESA, temp);
                } else {
                    send_message(client_sock, MSG_ERR, "Classifica non disponibile");   //Invio messaggio di errore
                }
            pthread_mutex_unlock(&pausa_gioco_mutex);
            break;

            case MSG_FINE:      
            printf("[SERVER] Il client %s ha richiesto la disconnessione.\n", utente->username);
            
            pthread_mutex_lock(&clients_mutex);
            deleteClient(clients,pthread_self());                                       //Richiamo alla funzione per elimina client
            pthread_mutex_unlock(&clients_mutex);    
            
            //lista.count --;
            utente->isPlayer = 0;                                                       //Disattivo utente
            close(client_sock);                                                         //Chiusura socket
            printf("[Handler] thread terminato\n");
            pthread_exit(NULL);                                                         //Terminazione

            case MSG_CANCELLA_UTENTE:                                                   //Richiesta cancellazione utente
            printf("[SERVER] Il client %s ha richiesto la cancellazione dell'utente.\n", utente->username);
                
                //Controllo se utente è attivo
                if (utente->isPlayer == 0) {
                    send_message(client_sock, MSG_ERR, "Devi essere loggato per disconnetterti");
                    break;
                }
            
                // Verifica che il nome utente fornito corrisponda all'utente del thread
                if (strcmp(utente->username, client_message.data) != 0) {
                    send_message(client_sock, MSG_ERR, "Nome utente non corrisponde.");
                    break;
                }
        
                //Verifica che il thread che richiede la cancellazione sia lo stesso che ha creato l'utente.
                if (pthread_equal(utente->thread_id, pthread_self()) == 0) {
                    send_message(client_sock, MSG_ERR, "Non puoi cancellare un altro utente.");
                    break;
                }
        
            // Cancellazione dell'utente
            pthread_mutex_lock(&lista_mutex);
            elimina_giocatore(&lista, client_message.data);
            send_message(client_sock, MSG_FINE, "Utente cancellato correttamente");
            pthread_mutex_unlock(&lista_mutex);
        
            //Cancellazione del Client
            pthread_mutex_lock(&clients_mutex);
            deleteClient(clients, pthread_self());
            pthread_mutex_unlock(&clients_mutex);

            shutdown(client_sock, SHUT_RDWR);                                               //Chiusura del socket
            close(client_sock);
            printf("[Handler] Cancellazione avvenuta con successo\n");
            pthread_exit(NULL);                                                             //Terminazione
            break;


            case MSG_LOGIN_UTENTE:                                                          //Richiesta login utente
                //Controllo se corrisponde l'username
                if (login_utente(client_sock, &lista, client_message.data) == 0){
                    pthread_mutex_lock(&clients_mutex);
                    //Inizializzo current alla testa del client
                    Client* current = clients->head;
                    //Scansione
                    while (current != NULL) {
                        if (pthread_equal(current->thread_id, pthread_self())) {            //Controllo se il tid corrisponde  
                            current->username = strdup(client_message.data);                //Copio l'username
                            current->isPlayer = 1;                                          //Attivo il client    
                            break;
                        }
                        current = current->next;                                            //Prossimo giocatore
                    }
                    pthread_mutex_unlock(&clients_mutex);
                }
            break;

            case MSG_POST_BACHECA:                                                          //Richiesta post dei messaggi sulla bacheca
                //Controllo se utente attivo
                if (utente->isPlayer == 0){
                    send_message(client_sock, MSG_ERR, "Devi essere loggato per postare un messaggio");
                    break;
                }
                if (add_message(client_message.data, utente->username)){                    //Controllo se messaggio aggiunto, in tal caso messaggio di ok
                    send_message(client_sock, MSG_OK, "Messaggio postato con successo");
                } else {
                    send_message(client_sock, MSG_ERR, "Errore nel postare il messaggio");
                }
            break; 
    
            case MSG_SHOW_BACHECA:                                                          //Richiesta di visualizzazione bacheca
                //Controllo se giocatore è attivo
                if (utente->isPlayer == 1){
                    pthread_mutex_lock(&mess);
                    char* buffer = show_bacheca();                                          
                    pthread_mutex_unlock(&mess);
                    send_message(client_sock, MSG_SHOW_BACHECA, buffer);                    //Mando bacheca
                } else {
                    send_message(client_sock, MSG_ERR, "Loggati  per vedere i messaggi in bacheca");
                }    
            break;

            default:                                                                        //Qualsiasi altra richiesta
            send_message(client_sock, MSG_ERR, "Comando non valido");
            break;
        }
    }
}

/*********/
/*SCORER*/
/*******/
void *scorer() {
    //COSTRUZIONE CLASSIFICA
    pthread_mutex_lock(&classifica_mutex);
        int max_length = 1024;    
        if (classifica){
            free(classifica);
        } 
        classifica = malloc(max_length);                                //Alloca memoria per la classifica
        
        if (!classifica) {
            printf("Errore di allocazione memoria per classifica\n");
            pthread_mutex_unlock(&classifica_mutex);
            return NULL;
        }
        memset(classifica,0,max_length);                                //Azzera la classifica
    pthread_mutex_unlock(&classifica_mutex);

    printf("Scorer in esecuzione\n");
    pthread_mutex_lock(&lista_mutex);
    int num_giocatori = lista.count;
    pthread_mutex_unlock(&lista_mutex);
    
        //Controllo numero di giocatori
        if (num_giocatori == 0) {
            printf("Nessun giocatore registrato.\n");
            return NULL;
        }

        // Alloca array di puntatori per i giocatori
        giocatore **scorerVector = malloc(num_giocatori * sizeof(giocatore *));
        //Allocazione avvenuta erroneamente
        if (!scorerVector) {
            printf("Errore di allocazione memoria\n");
            return NULL;
        }

    pthread_mutex_lock(&lista_mutex);
    giocatore *current = lista.head;                                    //Inizializzo current alla testa della lista
        
        //Copia i puntatori ai giocatori nell'array scorerVector.
        for (int i = 0; i < num_giocatori; i++) {
            if (current == NULL) break;
            scorerVector[i] = current;
            current = current->next;
        }

    pthread_mutex_unlock(&lista_mutex);

    // Ordina i giocatori per punteggio
    qsort(scorerVector, num_giocatori, sizeof(giocatore *), compare_score);

    classifica[0] = '\0';                                               //Inizializza la stringa di classifica
    int offset = 0;

        for (int i = 0; i < num_giocatori; i++) {
            const char *username_safe = scorerVector[i]->username ? scorerVector[i]->username : "Sconosciuto";
            int punteggio_corrente =  prendi_punteggi(&lista, scorerVector[i]->tid);                                //Ottengo punteggio del giocatore
            // Formatta la riga della classifica e la aggiunge alla stringa 'classifica'
            int written = snprintf(classifica + offset, max_length - offset,"%d. %s - %d punti\n", i + 1, username_safe, punteggio_corrente);

            if (written < 0 || written >= max_length - offset){                                                     //Controllo presenza di errori nella formattazione
                printf("Errore nella generazione della classifica\n");
                break;
            }
            offset += written;                                                                                      //Aggiorna offset per la prossima classifica
        }

    classifica[offset] = '\0';

    printf("Classifica generata:\n%s\n", classifica);                   //Stampa la classifica
    free(scorerVector);                                                 //Libera scoreVector
    reset_punteggi();                                                   //Reset dei punteggi
    return NULL;
}

/********************/
/*GESTIONE DEL GIOCO*/
/*******************/
void *game(void *arg) {
    int round = 0;
    while (1) {
    pthread_mutex_lock(&lista_mutex);
    
        //Controllo se ci sono giocatori registrati
        if (lista.count == 0){
            pthread_mutex_unlock(&lista_mutex);
            printf("Nessun giocatore registrato, attesa...\n");
            
            while (lista.count == 0){
                time(&tempo_iniziale);
            }

        pthread_mutex_lock(&lista_mutex);
        }

    pthread_mutex_unlock(&lista_mutex);

    printf("[CLASSIFICA] Classifica_inviata resettata a 0\n");
    classifica_inviata = 0;
    time(&tempo_iniziale);
    alarm(durata_pausa);
    printf("-----------------------------------------------------------------------\n");
    printf("Il round è terminato, inizierà tra: %d secondi\n", durata_pausa);

    pthread_mutex_lock(&clients_mutex);
    //Reset paroleTrovate
    Client * player1 = clients->head;
        while (player1 != NULL){
            player1->paroleTrovate = NULL;
            player1 = player1->next;
        } 
    pthread_mutex_unlock(&clients_mutex);
    printf("[LISTA_PAROLE_TROVATE]: Lista parole resettata per il nuovo round\n");

        //Se round non pronto, prepara e carica la matrice
        if (round == 0) {
            FILE *file = fopen("../Matrici.txt", "rb");
            if (file == NULL){
                perror("Errore nell'apertura del file");
                return NULL;
            }
            Carica_MatricedaFile(file, matrice);
            fclose(file);
            round = 1;
        }

        // Attendi che la pausa finisca
        while (pausa_gioco) {
            sleep(1);
        }
        
    //Se alla fine della pausa non ci sono giocatori, ripeti il ciclo
    pthread_mutex_lock(&lista_mutex);
        if (lista.count == 0) {
            pthread_mutex_unlock(&lista_mutex);
            pausa_gioco = 1;
            continue;
        }
    pthread_mutex_unlock(&lista_mutex);

    //INIZIO DEL GIOCO
    time(&tempo_iniziale);
    struct tm *timeinfo = localtime(&tempo_iniziale);
    char *orario_completo = asctime(timeinfo);
    char orario[9];
    snprintf(orario, sizeof(orario), "%.8s", orario_completo + 11);
    alarm(durata_partita);
    printf("-----------------------------------------------------------------------\n");
    printf("La partita è iniziata alle %s con %d giocatori\n", orario_completo, lista.count);

    // Invio matrice ai giocatori
    pthread_mutex_lock(&clients_mutex);
    Client *current = clients->head;
        //Scansione
        while (current != NULL) {
            if (current->isPlayer == 1) {                                               //Controlla che il client sia attivo
                printf("[MATRICE]Invio a %s\n", current->username);
                invio_matrice(current->fd, matrice);
                char *temp = calcola_tempo_rimanente(tempo_iniziale, durata_partita);
                send_message(current->fd, MSG_TEMPO_PARTITA, temp);
            }
            current = current->next;
        }
    pthread_mutex_unlock(&clients_mutex);

     // ATTENDI LA FINE DEL ROUND
        while (!pausa_gioco) {
            sleep(1);
        }

        printf("[SIGUSR2]Chiamando sigusr2_classifica_handler\n");
        sigusr2_classifica_handler(SIGUSR2);
        pausa_gioco = 1;
        round = 0;
    }
}


//Funzione per la disconnessione attività dopo un periodo di tempo 
void *thread_func_activity() {
    while (1) {
        sleep(10);                                                                   
        time_t now = time(NULL);

        pthread_mutex_lock(&clients_mutex);
        Client *prev = NULL;
        Client *current = clients->head;

        while (current != NULL) {

            //Controllo se la differenza dell'ultima attività dell'utente supera i 2 minuti
            if (difftime(now, current->last_activity) > TIMEOUT_MINUTES * 10) { 
                printf("Il client %s è inattivo da troppo tempo e verrà disconnesso.\n", current->username ? current->username : "Client non registrato");
                int retvalue;
                writef(retvalue,"[SERVER]: Inattività da due minuti\n");
                elimina_thread(clients, current->thread_id, &clients_mutex);                //Eliminazione del thread
                
                // Ripristina il puntatore per continuare la scansione della lista
                if (prev == NULL) {
                    current = clients->head;
                } else {
                    current = prev->next;
                }
                continue;
            }
            prev = current;
            current = current->next;
        }
        pthread_mutex_unlock(&clients_mutex);
    }
}

/*******/
/*MAIN*/
/*****/
int main(int argc, char *argv[]){
    Dizionario = create_node();
    Load_Dictionary(Dizionario, DIZIONARIO);


    //SOCKET
    int server_sock;
    struct sockaddr_in server_addr;
    
    // Segnali
    signal(SIGINT, sigint_handler);
    signal(SIGUSR2, sigusr2_classifica_handler);
    signal(SIGALRM, alarm_handler);


    //Controllo del numero dei parametri passati
    if (argc < 3){
        printf("Errore: Numero errato di parametri passiti, aspettati : 3, ricevuti: %d \n" , argc);
        return 0;
    }

    //Creazione del socket
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    //Se la connessione fallisce, invio messaggio di errore
    if (server_sock < 0){
        perror("Socket creation failed");
        return 1;
    }

    //Inizializzazione dell'indirizzo del server
    memset(&server_addr, 0, sizeof(server_addr));     // Azzerare la struttura
    server_addr.sin_family = AF_INET;                 // IPv4
    server_addr.sin_addr.s_addr = inet_addr(argv[1]); // Indirizzo server
    server_addr.sin_port = htons(atoi(argv[2]));      // Porta del server
    
    //Creo ed inizializzo la lista utenti
    clients = create();

    //BIND() assegna l'indirizzo specificato nella struttura server_addr al socket server_sock
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        //Fallisce: messaggio di errore e chiusura socket
        perror("Bind fallita");
        close(server_sock);
        return 1;
    }

    // Se funziona il bind, il socket si mette in ascolto per poter accettare connessioni in entrata
    if (listen(server_sock, 32) < 0){
        // Fallisce: messaggio di errore e chiusura socket
        perror("Listen fallita");
        close(server_sock);
        return 1;
    }

    printf("In attesa di connessioni...\n");
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    pthread_t partita;
    int retvalue;
    matrice = generateMatrix();

    //
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--disconnetti-dopo") == 0 && i + 1 < argc) {
            printf("Timeout client impostato a %d secondi\n", TIMEOUT_MINUTES);
        }
    }

    //Creazione del thread di gioco
    SYST(retvalue, pthread_create(&partita, NULL, game, NULL), "nella creazione del thread di gioco");

    //Creazione del thread per la disconnessione
    pthread_t activity_thread;
    SYST(retvalue, pthread_create(&activity_thread, NULL, thread_func_activity, NULL), "nella creazione del thread di attività");

    while (1){   
        //Accetta la connessione
        int client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_len);
        printf("Connessione accettata\n");
        pthread_t thread_id;

        // Creazione del thread a cui passo il fd associato al socket del client
        if (pthread_create(&thread_id, NULL, thread_func, &client_sock) != 0){
            // Se fallisce la creazione del thread, stampa un messaggio di errore e termina
            perror("Failed to create thread");
            return 1;
        }
    }

}   