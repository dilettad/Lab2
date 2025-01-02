#include <stdio.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>

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
//void calcola_tempo_rimanente(time_t tempo_iniziale, int durata);

int pausa_gioco = 0; //Gioco
// La partita dura 5 minuti quindi 300s
int durata_partita = 300;
//La pausa della partita dura 1.5 minuti
int durata_pausa = 90;
int punteggio = 0; // Devo aggiungere il punteggio tramite le mutex?
int classifica = 0; // Classifica non disponibile
int scorer = 0; // Scorer

// MUTEX
pthread_mutex_t pausa_gioco_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t matrix_mutex;
pthread_mutex_t lista_mutex;
pthread_mutex_t scorer_mutex;
pthread_mutex_t scorer_cond;

// Handler dei segnali
void alarm_handler(int sig){
    //Quando scade il tempo se il gioco era in pausa, cambia lo stato in modo da ricominciare
    if(pausa_gioco == 1){
        pthread_mutex_lock(&pausa_gioco);
        pausa_gioco = 0;
        printf("Il gioco è in corso.\n");
        pthread_mutex_unlock(&pausa_gioco);
    }
    else{
        //Quando scade il tempo, il gioco si ferma e cambia lo stato
        int retvalue;
        pthread_mutex_lock(&pausa_gioco);
        pausa_gioco = 1;
        printf("Il gioco è in pausa. \n");
        pthread_mutex_unlock(&pausa_gioco);
        //Invio del segnale a tutti i thread giocatori notificando che il gioco è finito
        if(listaGiocatori.count > 0){
            scorer = 1;
            invia_SIG(&lista, SIGUSR1, lista_mutex);
            //Creazione dello scorer
            SYSC(retvalue, pthread_create(&scorer_tid, NULL, scorer, NULL), "Errore nella pthread_create dello scorer");
        }
    }
}

void sigusr1_handler(int sig){
    // Quando ricevo il segnale SIGUSR1, il thread giocatore deve inviare il punteggio e l'username al server
    pthread_mutex_lock(%lista_mutex);
    // Prende il punteggio e l'username della lista dei giocatori
    char* username = get_username (%lista, pthread_self());
    if (username == NULL) {
        pthread_mutex_unlock(&lista_mutex);
        return;
    }
    int punteggio = get_punteggio(&lista, pthread_self());
    aggiorna_punteggio (&lista, username, 0);
    pthread_mutex_unlock(&lista_mutex);
    //mandare il punteggio e l'username allo scorer
    pthread_mutex_lock(&scorer_mutex);
    if (pushRisList(&scoreList, username, punteggio) != 0){
        free(username); // Libera la memoria se l'aggiunta fallisce
    }
    pthread_cond_signal(&scorer_cond);
    pthread_mutex_unlock(&scorer_mutex);
} 

void sigusr2_handler(int sig){
    // Quando ricevo il segnale SIGUSR2, lo scorer manda la classifica a tutti i thread giocatori
    scorer = 0;
    sendClassifica(&lista, pthread_self(), lista_mutex, classifica, tempo_iniziale, durata_pausa);
}

// Funzione per terminazione del server
void sigint_handler(int sig){
    int retvalue;
    // Scorro la lista dei thread attivi, in modo da chiuderli
    if (listaGiocatori.count != 0){
        pthread_mutex_lock(&lista_mutex);
        for (int i = 0; i < listaGiocatori.count; i++){
        // Invia un messaggio di chiusura a ciascun giocatore
            if (send_message(listaGiocatori.array[i].client_sock, "Il gioco è finito.\n") < 0) {
                perror("Errore nell'invio del messaggio di chiusura");
            }
            // Cancella il thread del giocatore
            if (pthread_cancel(listaGiocatori.array[i].thread_id) != 0) {
                perror("Errore nella cancellazione del thread");
            }
        pthread_mutex_unlock(&lista_mutex);
    }
   // Distruggi lista dei giocatori 
   distruggi_lista(&listaGiocatori);
   // Chiudi socket
   if (close(server_fd) == -1) {
   perror("Errore nella chiusura del socket del server");
}
printf("Chiusura del server \n");
exit(EXIT_SUCCESS);
}
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
                    char *temp = calcola_tempo_rimanente(tempo_iniziale, durata_partita);
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
                    if(esiste_paroleTrovate(listaParoleTrovate, data)){ //SISTEMARE 
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

// Funzione per gestire lo scorer
typedef struct{
    char* username;
    int punteggio;
} risGiocatore;


 /* QUESTA FUNZIONE NON FUNZIONA 
void *scorer(void* arg){
    printf("Scorer in esecuzione \n");
    //Registrazione dei segnali
    signal(SIGINT, sigusr2_handler);
    pthread_mutex_lock(&lista_mutex);
    int num_giocatori = listaGiocatori.count
    pthread_mutex_unlock(&lista_mutex);

    risGiocatore scorerVector[MAX_CLIENTS];

    for (int i = 0, i < num_giocatori; i++){
        pthread_mutex_lock(&scorer_mutex);
        while (scoreList ==  NULL){
            pthread_cond_wait(&scorer_cond, &scorer_mutex);
        }
        risGiocatore *temp = PopRisList (&scoreList);
        if (temp == NULL){
            pthread_mutex_unlock(&scorer_mutex);
            return NULL;
        }
        scorerVector[i].username = temp -> username;
        scorerVector[i].punteggio = temp -> punteggio;
        free(temp);
        pthread_mutex_unlock(&scorer_mutex);
    }
    compare_score(scorerVector, num_giocatori, sizeof(risGiocatore), &compare_score_func);
    pthread_mutex_lock(&scorer_mutex);
    
    invia_SIG(&listaGiocatori, SIGINT, lista_mutex);
    return NULL;
}
*/
// FUNZIONE AGGIUSTATA DA CHAT
void *scorer(void *arg) {
    printf("Scorer in esecuzione\n");

    // Registrazione dei segnali
    signal(SIGUSR2, sigusr2_handler); // Cambiato SIGINT in SIGUSR2 per evitare conflitti

    // Prendo il numero di giocatori registrati
    pthread_mutex_lock(&lista_mutex);
    int num_giocatori = listaGiocatori.count; // Assicurati che listaGiocatori sia definito correttamente
    pthread_mutex_unlock(&lista_mutex);

    risGiocatore scorerVector[MAX_CLIENTS];

    // Ciclo per raccogliere i risultati
    for (int i = 0; i < num_giocatori; i++) {
        pthread_mutex_lock(&scorer_mutex);
        while (scoreList == NULL) {
            pthread_cond_wait(&scorer_cond, &scorer_mutex);
        }

        risGiocatore *temp = PopRisList(&scoreList);
        if (temp == NULL) {
            pthread_mutex_unlock(&scorer_mutex);
            fprintf(stderr, "Errore: impossibile popolare la lista dei risultati.\n");
            return NULL; // Esci in caso di errore
        }

        scorerVector[i].username = temp->username; // Assicurati che username sia allocato correttamente
        scorerVector[i].punteggio = temp->punteggio;
        free(temp); // Libera la memoria allocata per temp
        pthread_mutex_unlock(&scorer_mutex);
    }

    // Ordinamento dei risultati
    qsort(scorerVector, num_giocatori, sizeof(risGiocatore), compare_score_func);

    // Invio segnale a tutti i thread giocatori notificandoli che possono prelevare la classifica
    invia_SIG(&listaGiocatori, SIGUSR2, lista_mutex); // Cambiato SIGINT in SIGUSR2

    printf("Classifica pronta. %d giocatori registrati.\n", num_giocatori);
    return NULL;
} 

// Funzione che gestisce il thread di gioco - aggiustata da chat
void* game(void* arg){
    printf("Giocatore in esecuzione\n");
    int round = 0;
    time_t tempo_iniziale;

    while(1){
        pthread_mutex_lock(&lista_mutex);
        // Attesa fino a quando non ci sono giocatori registrati
        while (listaGiocatori.count == 0){
            printf("Nessun giocatore registrato. Attesa...\n");
            pthread_cond_wait(&listaGiocatori.cond, &listaGiocatori.mutex);
        }   
        pthread_mutex_unlock(&lista_mutex);
        // Inizia la pausa
        sleep(1);
        alarm(durata_pausa);
        printf("La partita è in pausa, inizierà: %d secondi\n", durata_pausa);
        
        //PREPARAZIONE DEL ROUND
        if (round == 0){
            pthread_mutex_lock(&matrix_mutex);
           if(Carica_MatricedaFile("file-txt", matrice !=0)){
              perror("Errore nel caricamento della matrice");
           }    
            pthread_mutex_unlock(&matrix_mutex);
            round = 1;
        }
        // Se alla fine della pausa non ci sono giocatori registrati, si ripete il ciclo
        pthread_mutex_lock(&lista_mutex);
        if (listaGiocatori.count == 0){
            pausa_gioco = 1;
            pthread_mutex_unlock(&lista_mutex);
            continue;
        }
        pthread_mutex_unlock(&lista_mutex);    
       
        // Inizia la pausa
        time_(&tempo_iniziale);
        alarm(durata_partita);
        printf("La partita è iniziata, terminerà: %d secondi con %d giocatori\n" , time_t tempo_iniziale); // Perchè non mi prende il tempo iniziale?
        pthread_mutex_lock(&lista_mutex);
        giocatore* current = listaGiocatori.head;
        while(current != NULL){
            char* temp = calcola_tempo_rimanente(tempo_iniziale, durata_partita);
            if (send_message(current -> client_fd, strlen(temp), MSG_TEMPO_PARTITA, temp)< 0){
                perror("Errore nell'invio del messaggio");
            }
            current = current -> next;
    }
    pthread_mutex_unlock(&lista_mutex);
    while (!pausa_gioco){
        // Attesa
    }
    //Reset dalla pausa per il prossimo round 
    pausa_gioco = 0;
    round = 0;
    }    
}

int main(int argc, char* argv[]) {
    int server_sock;
    struct sockaddr_in server_addr;
    //char message [128];
    
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



}

//int pausa_gioco = 1; // 1 = si, 0 = no



// Invio della matrice e del tempo rimanente in base alla fase del gioco in cui è il giocatore
void invio_matrice(int client_fd, char matrix [MATRIX_SIZE][MATRIX_SIZE]){
    int length = MATRIX_SIZE * MATRIX_SIZE;
    char data[length];
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            data [i * MATRIX_SIZE + j] = matrix[i][j];
        }
    }    
    printf("Invio matrice al client %d\n", client_fd);
    send_message(client_fd, MSG_MATRICE, data);
}



/*MAIN
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