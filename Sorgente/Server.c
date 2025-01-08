#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>

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

void invio_matrice(int client_fd, char matrix[MATRIX_SIZE][MATRIX_SIZE]);
// char* calcola_tempo_rimanente(time_t tempo_iniziale, int durata_partita);

int pausa_gioco = 0; //Gioco
int durata_partita = 300; // La partita dura 5 minuti quindi 300s
int durata_pausa = 90; //La pausa della partita dura 1.5 minuti
int punteggio = 0; 
int classifica = 0; // Classifica non disponibile
int scorer = 0; // Scorer
int server_fd;


// MUTEX
pthread_mutex_t pausa_gioco_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t matrix_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lista_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t scorer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t scorer_cond;
pthread_t scorer_tid;
listaGiocatori lista; // Lista giocatori
time_t tempo_iniziale;
pthread_mutex_t game_mutex, game_cond;

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
        int retvalue;
        pthread_mutex_lock(&pausa_gioco_mutex);
        pausa_gioco = 1;
        printf("Il gioco è in pausa. \n");
        pthread_mutex_unlock(&pausa_gioco_mutex);
        
        //Invio del segnale a tutti i thread giocatori 
        if(lista.count > 0){
            scorer = 1;
            invia_SIG(&lista, SIGUSR1, lista_mutex);
            int retvalue = pthread_create(&scorer_tid, NULL, scorer, NULL);
            if (retvalue != 0) {
                perror("Errore nella pthread_create dello scorer");
            }    
        }
    }
}
signal(SIGUSR2, sig_classifica);
/*Funzione per invio della classifica
void sig_classifica(int sig){
    pthread_mutex_lock(&lista_mutex);

    if(listaGiocatori == NULL){
        printf("Nessun giocatore registrato, classifica non disponibile \n");
        pthread_mutex_unlock(&lista_mutex);
        return;
    }

    giocatore* current = listaGiocatori.head;
    while (current != NULL){
        send_message(current ->client_fd, strlen(classifica), MSG_PUNTI_FINALI, classifica);
        current = current -> next;
    }
    pthread_mutex_unlock(&lista_mutex);
} 
*/
// Funzione per invio della classifica
void sig_classifica(int sig){
    pthread_mutex_lock(&lista_mutex);
    // Controllo se ci sono giocatori registrati
    if (listaGiocatori.head == NULL){
        printf("Nessun giocatore registrato, classifica non disponibile \n");
        pthread_mutex_unlock(&lista_mutex);
        return;
    }
    scorer = 0;
    sendClassifica(&listaGiocatori, pthread_self(), lista_mutex, classifica, tempo_iniziale, durata_pausa);
    pthread_mutex_unlock(&lista_mutex);
}

signal(SIGINT, sigint_handler);
// Funzione per la chiusura del server
void sigint_handler(int sig) {
    int retvalue;

    // Scorro la lista dei giocatori, in modo da chiuderli
    if (listaGiocatori.count != 0) {
        pthread_mutex_lock(&lista_mutex);
        
        giocatore* current = listaGiocatori.head; // Inizializza il puntatore alla testa della lista
        while (current != NULL) { // Ciclo sulla lista
            // Invia un messaggio di chiusura a ciascun giocatore
            if (send_message(current->client_sock, "Il gioco è finito.\n") < 0) {
                perror("Errore nell'invio del messaggio di chiusura");
            }
            // Cancella il thread del giocatore
            if (pthread_cancel(current->thread_id) != 0) {
                perror("Errore nella cancellazione del thread");
            }
            current = current->next; // Passa al prossimo giocatore
        }
        
        pthread_mutex_unlock(&lista_mutex);
    }

    // Distruggi lista dei giocatori 
    distruggi_lista(listaGiocatori); // Assicurati che questa funzione gestisca correttamente la lista collegata

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
    char parola = receive_message(client_sock);
    int retvalue;
    while(1){
        message client_message = receive_message(client_sock);
        writef(retvalue,client_message.data);
        switch (client_message.type){
            case MSG_MATRICE:
                if(pausa_gioco == 0){
                    // Gioco quindi invio la matrice attuale e il tempo di gioco rimanente
                    invio_matrice(client_sock, matrice);
                    char* temp =  calcola_tempo_rimanente(tempo_iniziale, int durata_partita);
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
                   // Inserire una lista parole per confrontare
                    if(esiste_paroleTrovate(paroleTrovate, data)){ //SISTEMARE 
                        send_message(client_sock, 1, MSG_PUNTI_PAROLA, "0");
                        break;
                    }
                //Controllo se parola è in matrice
                    else if(!trovaParola(matrice, data)){
                        send_message(client_sock, MSG_ERR, "Parola nella matrice non trovata");
                        break;
                    }
                   
                //Controllo se parola è nel dizionario
                    else if(!search_Trie(root, data)){
                        send_message(client_sock, MSG_ERR, "Parola nel dizionario non trovata");
                        break;
                    }
                // Se i controlli hanno esito positivo, allora aggiungo parola alla lista delle parole trovate
                else{
                    // Aggiungo la parola alla lista delle parole trovate
                    paroleTrovate = aggiungiParolaTrovata(listaParoleTrovate, data); //DA SISTEMARE LA LISTA
                    int puntiparola = strlen(data);
                    // Se la parola contiene "Q" con "u" a seguito, sottraggo di uno i punti
                    if (strstr(data,"Qu")){
                        puntiparola--;
                    } 
                    // Invio i punti della parola
                    send_message(client_sock, puntiparola, MSG_PUNTI_PAROLA, data);
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
                if(pausa_gioco == 1 && classifica == 1){
                    send_message(client_sock, strlen(classifica), MSG_PUNTI_FINALI, classifica);
                    char *temp = calcola_tempo_rimanente(tempo_iniziale, durata);
                    send_message(client_sock, strlen(temp), MSG_TEMPO_ATTESA, temp);
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

/* Funzione per gestire i thread giocatori 
void* gestisci_giocatore(void* args){
    int client_fd = *(int*)args;
    int punteggio;
    char* username = NULL;
    char MSG;

    while(1){
        char *data = receive_message(client_fd, &MSG);
        if (MSG == MSG_CHIUSURA_CLIENT);
        rimuovi_thread(&listaGiocatori)

    }


}   
*/ 

pthread_mutex_t classifica_mutex;
void* scorer(void *arg) {

    printf("Scorer in esecuzione\n");

    // Prendo il numero di giocatori registrati
    pthread_mutex_lock(&lista_mutex);
    int num_giocatori = listaGiocatori.count; 
    pthread_mutex_unlock(&lista_mutex);

    giocatore  scorerVector[MAX_CLIENTS];
    giocatore *current = listaGiocatori.head;
    
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
    memset(classifica, 0, sizeof(classifica)); //Svuota la classifica
    char msg[MAX_BUFFER];
    for (int i = 0; i < num_giocatori; i++){
        sprintf(msg, "%s %d\n", scorerVector[i].username, scorerVector[i].punteggio);
        strcat(classifica, msg, sizeof(classifica) - strlen (classifica) - 1 );
    }

    pthread_mutex_unlock(&classifica_mutex);
    for (int i = 0; i < num_giocatori; i++){
        free(scorerVector[i].username);
    }
    printf("Classifica pronta. %d giocatori registrati.\n", num_giocatori);
    // Invio segnale a tutti i thread giocatori notificandoli che possono prelevare la classifica
    invia_SIG(&listaGiocatori, SIGUSR2, lista_mutex); // Cambiato SIGINT in SIGUSR2
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
        while (listaGiocatori.count == 0) {
            printf("Nessun giocatore registrato. Attesa...\n");
            pthread_cond_wait(&listaGiocatori.cond, &listaGiocatori.mutex);
        }   
        pthread_mutex_unlock(&lista_mutex);

        // Inizia la pausa
        sleep(1);
        alarm(durata_pausa);
        printf("La partita è in pausa, inizierà tra: %d secondi\n", durata_pausa);

        // Preparazione del round 
        if (round == 0) {
            pthread_mutex_lock(&matrix_mutex);
            if (Carica_MatricedaFile("file-txt", matrice) != 0) {
                perror("Errore nel caricamento della matrice");
                pthread_mutex_unlock(&matrix_mutex);
                continue;  // Salta l'inizio del round se non riesce a caricare la matrice
            }    
            pthread_mutex_unlock(&matrix_mutex);
            round = 1;
        }

        // Se alla fine della pausa non ci sono giocatori registrati, si ripete il ciclo
        pthread_mutex_lock(&lista_mutex);
        if (listaGiocatori.count == 0) {
            pausa_gioco = 1;
            pthread_mutex_unlock(&lista_mutex);
            continue;  // Riprende il ciclo se non ci sono giocatori
        }
        pthread_mutex_unlock(&lista_mutex);    

        // Inizio del round di gioco
        tempo_iniziale = time(NULL);
        alarm(durata_partita);
        printf("La partita è iniziata, terminerà tra: %d secondi con %d giocatori\n", durata_partita, listaGiocatori.count); 

        // Invio il tempo rimanente a ciascun giocatore
        pthread_mutex_lock(&lista_mutex);
        giocatore* current = listaGiocatori.head;
        while (current != NULL) {
            char* temp = calcola_tempo_rimanente(tempo_iniziale, durata_partita);
            if (send_message(current->client_fd, strlen(temp), MSG_TEMPO_PARTITA, temp) < 0) {
                perror("Errore nell'invio del messaggio");
            }
            current = current->next;
        }
        pthread_mutex_unlock(&lista_mutex);

        // Attendere che la partita finisca (usando condizione di attesa)
        pthread_mutex_lock(&game_mutex);
        while (!pausa_gioco) {
            pthread_cond_wait(&game_cond, &game_mutex); // Attendi la fine della partita o la pausa
        }
        pthread_mutex_unlock(&game_mutex);

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


    cella** matrice = generateMatrix();
        // Libera la memoria
    for (int i = 0; i < MATRIX_SIZE; i++) {
        free(matrice[i]);
    }
    free(matrice);

    return 0;
    InputStringa(matrice,"");


// int optarg;
// Inizializzazione parametri
struct parametri Parametri[] = {
    {"matrici", required_argument, 0, 'm'},
    {"durata", required_argument, 0, 'd'},
    {"seed", required_argument, 0, 's'},
    {"dizinario", required_argument, 0, 'x'},   
    {0,0,0,0} 
}; 

int opt;

 while((opt = getopt(argc, argv, "m:d:s:x:")) != -1){
        switch (opt) {
        //--matrici `e seguito dal nome del file dal quale caricare le matrici 
        case 'm':
            Parametri.data_filename = optarg;
            break;
        //--durata permette di indicare la durata del gioco in minuti. Se non espresso di default va considerato 3 minuti
        case 'd':
                Parametri.durata_partita = atoi(optarg);
                break;
        //--seed permette di indicare il seed da usare per la generazione dei numeri pseudocasuali
            case 's':
                Parametri.rnd_seed = atoi(optarg);
                break;
        //--diz permette di indicare il dizionario da usare per la verifica di leicità delle parole ricevute dal client.
            case 'x':
                Parametri.dizionario = optarg;
                break;
            default:
                fprintf(stderr, "Uso: %s nome_server porta_server [-m data_filename] [-d durata_in_minuti] [-s rnd_seed] [-x dizionario]\n", argv[0]);
                return 1;
        }
    }

}




/* MAIN Iniziale -> già inserito
int main(int argc, char* argv[]){
    //Controllo se il numero di parametri passati è corretto
    if(argc<3){
        //Errore
        printf("Errore: Numero errato di parametri passati");
        return 0; 
    }
    //Prendo il parametro nome_server
    char* nome_server = argv[1];
     //Controllo se il nome del server preso è quello corretto
    if(argv[1]!= "serverd"){
        printf("Errore:Nome del server errato");
        return 0;
    }
    //Prendo il parametro porta_server
    int porta_server = atoi(argv<[2]);
    
  //Parametri opzionali: [--matrici data_filename] [--durata durata_in_minuti] [--seed rnd_seed] [--diz dizionario] 

  
    //Gestione
    int opt;
    while((opt = getopt(argc, argv, "m:d:s:x:")) != -1){
        switch (opt) {
        //--matrici `e seguito dal nome del file dal quale caricare le matrici 
        case 'm':
            parametri.data_filename= optarg;
            break;
        //--durata permette di indicare la durata del gioco in minuti. Se non espresso di default va considerato 3 minuti
        case 'd':
                parametri.durata = atoi(optarg);
                break;
        //--seed permette di indicare il seed da usare per la generazione dei numeri pseudocasuali
            case 's':
                parametri.rnd_seed = atoi(optarg);
                break;
        //--diz permette di indicare il dizionario da usare per la verifica di leicità delle parole ricevute dal client.
            case 'x':
                parametri.dizionario = optarg;
                break;
            default:
                fprintf(stderr, "Uso: %s nome_server porta_server [-m data_filename] [-d durata_in_minuti] [-s rnd_seed] [-x dizionario]\n", argv[0]);
                return 1;
        }
    }

}   
*/