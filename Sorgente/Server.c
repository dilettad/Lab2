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

#define MAX_CLIENTS 32 
#define MAX_LENGTH_USERNAME 10 //Numero massimo di lunghezza dell'username
#define NUM_THREADS 5 //Numero di thread da creare
#define BUFFER_SIZE 1024 //dimensione del buffer
#define MATRIX_SIZE 4
#define DIZIONARIO "../Dizionario.txt"

void *scorer(void *arg);

// char* calcola_tempo_rimanente(time_t tempo_iniziale, int durata_partita);

int pausa_gioco = 0; //Gioco
int durata_partita = 300; // La partita dura 5 minuti quindi 300s
int durata_pausa = 90; //La pausa della partita dura 1.5 minuti
int punteggio = 0; 
char* classifica; // Classifica non disponibile
int server_fd;
cella** matrice;
paroleTrovate* listaParoleTrovate = NULL;
Trie* trie = NULL;

// MUTEX
pthread_mutex_t pausa_gioco_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t matrix_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lista_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t scorer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t scorer_cond;
pthread_t scorer_tid;
listaGiocatori lista; // Lista giocatori
time_t tempo_iniziale;



/*HO COPIATO PER FARLO COMPILARE, AGGIUSTALO*/
// Calcola tempo rimanente
// Char perchè mi restituisce un messaggio
char*  calcola_tempo_rimanente(time_t tempo_iniziale, int durata_partita) {
    time_t tempo_attuale = time(NULL);
    double tempo_trascorso = difftime(tempo_attuale, tempo_iniziale);
    int tempo_rimanente = durata_partita - (int)tempo_trascorso;
    // Se il tempo rimanente è minore di 0 allora vuol dire che il gioco è finito
    if (tempo_rimanente < 0) {
        return "Il gioco è già terminato\n";
    } 
    // Alloca memoria per il messaggio
    int length = snprintf(NULL, 0 , "Il tempo rimanente è: %d secondi\n", tempo_rimanente);
    char* messaggio = (char*)malloc(length + 1);
    // Verifica se l'allocazione è riuscita
    if (messaggio == NULL) {
        return "Errore di allocaione della memoria \n";
    }
    snprintf(messaggio, length+1, "Il tempo rimanente è: %d secondi\n", tempo_rimanente);  
    //return il messaggio
    return messaggio;  
}

//Funzione di invio classifica ai giocatori è necessaria?
void sendClassifica(listaGiocatori* lista, pthread_t tid, pthread_mutex_t lista_mutex, char* classifica, time_t tempo_iniziale, int durata_pausa_){
    pthread_mutex_lock(&lista_mutex);
    giocatore* current = lista->head;
    while(current != NULL){
        if(pthread_equal(current->tid, tid)){
            send_message(current->client_fd,MSG_PUNTI_FINALI, classifica);
            char *temp = calcola_tempo_rimanente(tempo_iniziale, durata_pausa_);
            send_message(current->client_fd, MSG_TEMPO_ATTESA, temp);
        }
        current = current->next;
    }
    pthread_mutex_unlock(&lista_mutex);
}

// Funzione per confrontare i punteggi dei giocatori -> qsort
int compare_score (const void *a, const void *b){
    giocatore *playerA = (giocatore*) a;
    giocatore *playerB = (giocatore*) b;
    return (playerB->punteggio - playerA->punteggio); // Ordinamento decrescente
}

//Funzione di invio segnali ai tutti i giocatori della lista
void invia_SIG(listaGiocatori* lista, int SIG, pthread_mutex_t lista_mutex){
    pthread_mutex_lock(&lista_mutex);
    giocatore* current = lista->head;
    while(current != NULL){
        pthread_kill(current->tid, SIG);
        current = current->next;
        }
        pthread_mutex_unlock(&lista_mutex);
}


// Handler dei segnali
// Funzione per cambiare stato del gioco
void alarm_handler(int sig){
    //Quando scade il tempo se il gioco era in pausa, cambia lo stato in modo da ricominciare
    if(pausa_gioco == 1){
        pthread_mutex_lock(&pausa_gioco_mutex);
        pausa_gioco = 0;
        printf("Il gioco è in corso.\n");
        pthread_mutex_unlock(&pausa_gioco_mutex);
    }
    else{
        //Quando scade il tempo, il gioco si ferma e cambia lo stato
        //int retvalue;
        pthread_mutex_lock(&pausa_gioco_mutex);
        pausa_gioco = 1;
        printf("Il gioco è in pausa. \n");
        pthread_mutex_unlock(&pausa_gioco_mutex);
        
        //Invio del segnale a tutti i thread giocatori 
        if(lista.count > 0){
            //scorer = 1;
            invia_SIG(&lista, SIGUSR1, lista_mutex);
            int retvalue = pthread_create(&scorer_tid, NULL, scorer, NULL);
            if (retvalue != 0) {
                perror("Errore nella pthread_create dello scorer");
            }    
        }
    }
}
//signal(SIGUSR2, sig_classifica);

// Funzione per invio della classifica
void sig_classifica(int sig){
    pthread_mutex_lock(&lista_mutex);
    // Controllo se ci sono giocatori registrati
    if (lista.head == NULL){
        printf("Nessun giocatore registrato, classifica non disponibile \n");
        pthread_mutex_unlock(&lista_mutex);
        return;
    }
    //scorer = 0;
    sendClassifica(&lista, pthread_self(), lista_mutex, classifica, tempo_iniziale, durata_pausa);
    pthread_mutex_unlock(&lista_mutex);
}

//signal(SIGINT, sigint_handler);
// Funzione per la chiusura del server
void sigint_handler(int sig) {
    int retvalue;

    // Scorro la lista dei giocatori, in modo da chiuderli
    if (lista.count != 0) {
        pthread_mutex_lock(&lista_mutex);
        
        giocatore* current = lista.head; // Inizializza il puntatore alla testa della lista
        while (current != NULL) { // Ciclo sulla lista
            // Invia un messaggio di chiusura a ciascun giocatore
            send_message(current->client_fd, MSG_FINE,"Il gioco è finito.\n");
            current = current->next; // Passa al prossimo giocatore
        }
        
        pthread_mutex_unlock(&lista_mutex);
    }

    // Distruggi lista dei giocatori 
    //distruggi_lista(lista); // Assicurati che questa funzione gestisca correttamente la lista collegata//implementa la funzione

    // Chiudi socket
    if (close(server_fd) == -1) {
        perror("Errore nella chiusura del socket del server");
    }
    
    printf("Chiusura del server \n");
    exit(EXIT_SUCCESS);
}


//SOCKET
// Funzione del thread
void* thread_func(void* arg) {
    // Dichiara un puntatore per il valore di ritorno
    int client_sock = *(int*)arg;

// Gestione dei comandi ricevuti dal client
// MSG_MATRICE: invia la matrice e il tempo rimanente o il tempo di pausa 
// MSG_PAROLA: controllo punti della parola in base ai caratteri, se presente nella matrice, nel dizionario e accredita punti, se già trovata 0
// MSG_REGISTRA_UTENTE: registra l'utente e controllo se già registrato
// MSG_PUNTI_FINALI: calcolo i punti totali
    int retvalue;
    int durata_partita;
    while(1){
        message client_message = receive_message(client_sock);
        writef(retvalue,client_message.data);
        switch (client_message.type){
            case MSG_MATRICE:
                if(pausa_gioco == 0){
                    // Gioco quindi invio la matrice attuale e il tempo di gioco rimanente
                    invio_matrice(client_sock, matrice);
                    char* temp =  calcola_tempo_rimanente(tempo_iniziale,durata_partita);
                    send_message(client_sock, MSG_TEMPO_PARTITA, temp);
                } else {
                    // Invio il tempo di pausa rimanente
                    char *temp = calcola_tempo_rimanente(tempo_iniziale, durata_pausa);
                    send_message(client_sock, MSG_TEMPO_ATTESA, temp);
                }
                break;

            case MSG_PAROLA:
                if (pausa_gioco==0){
                // Controllo se la parola è già stata trovata 
                   //Inserire una lista parole per confrontare
                    if(1/*esiste_paroleTrovate(paroleTrovate, data)*/){
                        send_message(client_sock,MSG_PUNTI_PAROLA, "0");
                        break;
                    }
                //Controllo se parola è in matrice
                    else if(!trovaParola(matrice, client_message.data)){
                        send_message(client_sock, MSG_ERR, "Parola nella matrice non trovata");
                        break;
                    }
                   
                //Controllo se parola è nel dizionario
                    else if(!search_Trie(client_message.data,trie)){
                        send_message(client_sock, MSG_ERR, "Parola nel dizionario non trovata");
                        break;
                    }
                // Se i controlli hanno esito positivo, allora aggiungo parola alla lista delle parole trovate
                else{
                    // Aggiungo la parola alla lista delle parole trovate
                    listaParoleTrovate = aggiungiParolaTrovata(listaParoleTrovate, client_message.data); //DA SISTEMARE LA LISTA
                    int puntiparola = strlen(client_message.data);
                    // Se la parola contiene "Q" con "u" a seguito, sottraggo di uno i punti
                    if (strstr(client_message.data,"Qu")){
                        puntiparola--;
                    } 
                    // Invio i punti della parolaù
                    char* messaggiopuntiparola = "diletta crea il messaggio";
                    send_message(client_sock, MSG_PUNTI_PAROLA,messaggiopuntiparola);
                    punteggio += puntiparola;
                }
            } else {
                // Invio il messaggio di errore
                send_message(client_sock, MSG_ERR, "Gioco in pausa!");
            }
            break;             
            
            case MSG_REGISTRA_UTENTE:
                //controllare nome utente
                send_message(client_sock, MSG_ERR, "Utente già registrato");
                break;

            case MSG_PUNTI_FINALI:
                if(pausa_gioco == 1 && classifica != NULL){
                    send_message(client_sock, MSG_PUNTI_FINALI, classifica);
                    char *temp = calcola_tempo_rimanente(tempo_iniziale, durata_partita);
                    send_message(client_sock, MSG_TEMPO_ATTESA, temp);
                }
                else{
                    send_message(client_sock, MSG_ERR, "Classifica non disponibile");
                }
                break;
            }
        send_message(client_sock,MSG_OK,"ciao diletta");
    }
    // Terminazione del thread con valore di ritorno
    pthread_exit(NULL);
}

pthread_mutex_t classifica_mutex;
void *scorer(void *arg) {

    printf("Scorer in esecuzione\n");

    // Prendo il numero di giocatori registrati
    pthread_mutex_lock(&lista_mutex);
    int num_giocatori = lista.count; 
    pthread_mutex_unlock(&lista_mutex);

    giocatore  scorerVector[MAX_CLIENTS];
    giocatore *current = lista.head;
    
    pthread_mutex_lock(&lista_mutex); 
    // Ciclo per raccogliere i risultati
    for (int i = 0; current != NULL && i< num_giocatori; i++) {
        scorerVector[i].username =strdup(current -> username); // copia l'username
        scorerVector[i].punteggio = current -> punteggio; //Copia punteggio
        current = current -> next; //Passa al prossimo giocatore
    }
    pthread_mutex_unlock(&lista_mutex);    

    // Ordinamento dei risultati
    qsort(scorerVector, num_giocatori, sizeof(giocatore), compare_score);

    pthread_mutex_lock(&classifica_mutex);
    classifica = NULL;
    char msg[256];
    for (int i = 0; i < num_giocatori; i++){
        sprintf(msg, "%s %d\n", scorerVector[i].username, scorerVector[i].punteggio);
        //strcat(classifica, msg,strlen (classifica) - 1 );
    }

    pthread_mutex_unlock(&classifica_mutex);
    for (int i = 0; i < num_giocatori; i++){
        free(scorerVector[i].username);
    }
    printf("Classifica pronta. %d giocatori registrati.\n", num_giocatori);
    // Invio segnale a tutti i thread giocatori notificandoli che possono prelevare la classifica
    invia_SIG(&lista, SIGUSR2, lista_mutex); // Cambiato SIGINT in SIGUSR2
    return NULL;
} 

// Funzione che gestisce il thread di gioco - aggiustata da chat

void* game(void* arg) {
    printf("Giocatore in esecuzione\n");
    int round = 0;
    time_t tempo_iniziale;

    while (1) {
        pthread_mutex_lock(&lista_mutex);
        // Attesa fino a quando non ci sono giocatori registrati
        /*NON ESISTE LA PROPRIETA COND NELLA LISTA*/
        // while (lista.count == 0) {
        //     printf("Nessun giocatore registrato. Attesa...\n");
        //     pthread_cond_wait(&lista.cond, &lista.mutex);
        // }   
        pthread_mutex_unlock(&lista_mutex);

        // Inizia la pausa
        sleep(1);
        alarm(durata_pausa);
        printf("La partita è in pausa, inizierà tra: %d secondi\n", durata_pausa);

        // PREPARAZIONE DEL ROUND
        if (round == 0) {
            pthread_mutex_lock(&matrix_mutex);
            FILE* file;//aprire il file "file-txt" e metterlo nella variabile file
            Carica_MatricedaFile(file, matrice);  
            pthread_mutex_unlock(&matrix_mutex);
            round = 1;
        }

        // Se alla fine della pausa non ci sono giocatori registrati, si ripete il ciclo
        pthread_mutex_lock(&lista_mutex);
        if (lista.count == 0) {
            pausa_gioco = 1;
            pthread_mutex_unlock(&lista_mutex);
            continue;  // Riprende il ciclo se non ci sono giocatori
        }
        pthread_mutex_unlock(&lista_mutex);    

        // Inizio del round di gioco
        tempo_iniziale = time(NULL);
        alarm(durata_partita);
        printf("La partita è iniziata, terminerà tra: %d secondi con %d giocatori\n", durata_partita, lista.count); 

        // Invio il tempo rimanente a ciascun giocatore
        pthread_mutex_lock(&lista_mutex);
        giocatore* current = lista.head;
        while (current != NULL) {
            char* temp = calcola_tempo_rimanente(tempo_iniziale, durata_partita);
           send_message(current->client_fd,MSG_TEMPO_PARTITA, temp);
            current = current->next;
        }
        pthread_mutex_unlock(&lista_mutex);
        //VARIABILI NON DICHIARATE
        // // Attendere che la partita finisca (usando condizione di attesa)
        // pthread_mutex_lock(&game_mutex);
        // while (!pausa_gioco) {
        //     pthread_cond_wait(&game_cond, &game_mutex); // Attendi la fine della partita o la pausa
        // }
        // pthread_mutex_unlock(&game_mutex);

        // Reset dalla pausa per il prossimo round
        pausa_gioco = 0;
        round = 0;
    }    
}




//
int main(int argc, char* argv[]) {
    int server_sock;
    struct sockaddr_in server_addr;
    //char message [128];

    // Definizione dei segnali 
    signal(SIGUSR1, alarm_handler);
    
    if(argc<3){
        //Errore
        printf("Errore: Numero errato di parametri passati");
        return 0; 
    }
    // Creazione del socket
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    // Inizializzazione dell'indirizzo del server
    memset(&server_addr, 0, sizeof(server_addr)); // Azzerare la struttura
    server_addr.sin_family = AF_INET; // IPv4
    server_addr.sin_addr.s_addr = inet_addr(argv[1]); // Indirizzo server
    server_addr.sin_port = htons(atoi(argv[2])); // Porta del server

    // BIND() assegna un indirizzo al socket
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind fallita");
        close(server_sock);
        return 1;
    }

    // Se funziona il bind, il socket ascolta
    if (listen(server_sock, 3) < 0) {
        perror("Listen fallita");
        close(server_sock);
        return 1;
    }

    printf("In attesa di connessioni...\n");

    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    while(1){
      // Accetta la connessione
        int client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_len);

        printf("Connessione accettata\n");
        
        /*read(client_sock, message, 128);
        printf("Client: %s\n", message);*/
        // Chiudi i socket
        
        //close(client_sock); // Chiudi il socket del client
        //close(server_sock); // Chiudi il socket del server
        //return 0;
        
        //THREAD
        pthread_t thread_id;

        // Creazione del thread a cui passo il fd associato al socket del client
        if(pthread_create(&thread_id, NULL, thread_func, &client_sock) != 0){
            perror("Failed to create thread");
            return 1;
        }

        //Aspetta che ogni thread termini e recupera il valore di ritorno
        for (int i = 0; i < NUM_THREADS; i++) {
            pthread_join(thread_id,NULL);
            //Stampa il valore di ritorno dal thread
            //free(ret); // Libera la memoria allocata
            printf("thread ucciso\n");
        }
        return 0;
    }


    matrice = generateMatrix();
        // Libera la memoria
    for (int i = 0; i < MATRIX_SIZE; i++) {
        free(matrice[i]);
    }
    free(matrice);

    return 0;
    InputStringa(matrice,"");



}