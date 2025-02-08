#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
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

int durata_partita = 8; // La partita dura 5 minuti quindi 30s
int durata_pausa = 5;    // La pausa della partita dura 1 minuti


#define MAX_CLIENTS 32
#define MAX_LENGTH_USERNAME 10 // Numero massimo di lunghezza dell'username
#define NUM_THREADS 5          // Numero di thread da creare
#define BUFFER_SIZE 1024       // dimensione del buffer
#define MATRIX_SIZE 4
#define DIZIONARIO "../Dizionario.txt"
#define TIMEOUT_MINUTES 2 // 2 minuti di inattività

typedef struct{
    char *matrix_file;
    float durata_partita;
    long seed;
    char *file_dizionario;
} Parametri;

void *scorer();

// char* calcola_tempo_rimanente(time_t tempo_iniziale, int durata_partita);

int pausa_gioco = 0;     // Gioco
//int durata_partita = 8; // La partita dura 5 minuti quindi 30s
// int durata_pausa = 5;    // La pausa della partita dura 1 minuti
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
// pthread_mutex_t mess = PTHREAD_MUTEX_INITIALIZER;
listaGiocatori lista; // Lista giocatori
Fifo *clients;        // Lista clienti
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
time_t tempo_iniziale;
Client *clients1;
/*VARIABILI PER GESTIONE GIOCO*/
int turno = 0;
pthread_mutex_t turno_mutex = PTHREAD_MUTEX_INITIALIZER;

int game_started = 0;
pthread_mutex_t game_started_mutex = PTHREAD_MUTEX_INITIALIZER;



// FUNZIONI PER IL 4 ADDENDUM
void *client_handler(void *arg){
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
char *calcola_tempo_rimanente(time_t tempo_iniziale, int durata_partita){
    time_t tempo_attuale = time(NULL);
    double tempo_trascorso = difftime(tempo_attuale, tempo_iniziale);
    double tempo_rimanente = durata_partita - (int)tempo_trascorso;

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
    snprintf(messaggio, length + 1, "Il tempo rimanente è: %d secondi\n", (int)tempo_rimanente);
    // return il messaggio
    return messaggio;
}

// Funzione di invio classifica ai giocatori
void sendClassifica(listaGiocatori *lista, pthread_t tid, char *classifica, time_t tempo_iniziale, int durata_pausa){
    pthread_mutex_lock(&lista_mutex);
    // Inizializza un puntatore alla testa della lista
    giocatore* current = lista->head;
    while (current != NULL){
        //if (pthread_equal(current->tid, tid)){                                                                   // Controllo ID del thread del giocatore uguale all'ID del thread passato come parametro
            send_message(current->client_fd, MSG_PUNTI_FINALI, classifica); // Invia la classifica finale al fd del giocatore
            char *temp = calcola_tempo_rimanente(tempo_iniziale, durata_pausa);
            send_message(current -> client_fd, MSG_TEMPO_ATTESA, temp);
            free(temp);
        //}
        current = current->next; // Passo al giocatore successivo
    }
    pthread_mutex_unlock(&lista_mutex);
}

// Funzione per confrontare i punteggi dei giocatori -> qsort
int compare_score(const void *a, const void *b){
    giocatore *playerA = (giocatore *)a;              // Puntatore a convertito in un puntatore della struttura giocatore
    giocatore *playerB = (giocatore *)b;              // Puntatore b convertito in un puntatore della struttura giocatore
    return playerB->punteggio - playerA->punteggio; // Confronto punteggi in ordine decrescente
}


// HANDLER DEI SEGNALI
// Funzione di invio segnali a tutti i giocatori della lista
void invia_SIG(listaGiocatori *lista, int SIG, pthread_mutex_t lista_mutex){
    pthread_mutex_lock(&lista_mutex);
    giocatore* current = lista->head; // Inizializza un puntatore alla testa della lista dei giocatori
    printf("Tid giocatore %ld \n", current->tid);
    printf("Invio segnale %d a tutti i giocatori \n", SIG);
    while (current != NULL){                                    // While finchè ci sono giocatori
        printf("Invio segnale %d al giocatore con tid %ld\n", SIG, current->tid);
        pthread_kill(current->tid, SIG); // Invia segnale SIG al thread current -> tid
        current = current->next;         // Passa al giocatore successivo
    }
    pthread_mutex_unlock(&lista_mutex);
}


// Funzione per invio della classifica
void sigusr2_classifica_handler(int sig){
    printf("Gestore del segnale SIGUSR2 chiamato \n");
    pthread_mutex_lock(&lista_mutex);
    // Controllo se ci sono giocatori registrati
    if (lista.head == NULL){ // Se la testa è vuoto
        printf("Nessun giocatore registrato, classifica non disponibile \n");
        pthread_mutex_unlock(&lista_mutex);
        return;
    }
    pthread_mutex_unlock(&lista_mutex);
    printf("Segnale SIGUSR2 ricevuto, la classifica è pronta per essere inviata ai giocatori\n");
    pthread_create(&scorer_tid, NULL, scorer, NULL);
    //sendClassifica(&lista, pthread_self(), lista_mutex, classifica, tempo_iniziale, durata_partita);
    //pthread_mutex_unlock(&lista_mutex);
}



// Funzione per la chiusura del server -> FUNZIONA
void sigint_handler(int sig){
    // Questa funzione chiude tutti quanti i client attivi (NON SOLO I GIOCATORI)
    pthread_mutex_lock(&clients_mutex);
    Client *current = clients->head;

    while (current != NULL)
    {
        Client *next = current->next;
        send_message(current->fd, MSG_SERVER_SHUTDOWN, "Il server è stato chiuso");
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

// CARICO IL DIZIONARIO IN MEMORIA
void Load_Dictionary(Trie *Dictionary, char *path_to_dict){    // APRO IL FILE TRAMITE IL PATH
    FILE *dict = fopen(path_to_dict, "r");
    // CREO UNA VARIABILE PER MEMORIZZARE LE PAROLE
    if (dict == NULL){
        fprintf(stderr, "Errore: impossibile aprire il file %s/n", path_to_dict);
        return;
    }
    char word[256];
    // LEGGO TUTTO IL FILE
    while (fscanf(dict, "%s", word) != EOF)
    {
        Caps_Lock(word); 
        insert_Trie(Dizionario, word);
      
    }
    return;
}

// SOCKET
//  Funzione del thread
void *thread_func(void *args){

    // Francesco: Aggiungere ad una lista il client con il suo fd

    // Dichiara un puntatore per il valore di ritorno
    int client_sock = *(int *)args;

    Client *utente = malloc(sizeof(Client));
    utente->username = NULL;
    utente->fd = client_sock;
    utente->score = 0;
    utente->next = NULL;
    utente->thread_id = pthread_self();
    push(clients, utente);
    int registra_bool = 0;

    int retvalue;

    while (1)
    {
        message client_message = receive_message(client_sock);
        writef(retvalue, client_message.data);
        switch (client_message.type)
        {
        case MSG_MATRICE:
            pthread_mutex_lock(&pausa_gioco_mutex);
            if (pausa_gioco == 0){
                // Gioco quindi invio la matrice attuale e il tempo di gioco rimanente
                stampaMatrice(matrice); 
                invio_matrice(client_sock, matrice); 
                char *temp = calcola_tempo_rimanente(tempo_iniziale, durata_partita);
                printf("Il tempo è %s", temp);
                send_message(client_sock, MSG_TEMPO_PARTITA, temp);
            } else {
                // Invio il tempo di pausa rimanente
                char *temp = calcola_tempo_rimanente(tempo_iniziale, durata_pausa);
                send_message(client_sock, MSG_TEMPO_ATTESA, temp);
            }
            pthread_mutex_unlock(&pausa_gioco_mutex);
            break;

        case MSG_PAROLA:
            pthread_mutex_lock(&pausa_gioco_mutex);
            if (pausa_gioco == 0){
                Caps_Lock(client_message.data);
                printf("La parola da cercare è %s\n", client_message.data);
                fflush(0);
                // Controllo se la parola è già stata trovata
                if (esiste_paroleTrovate(listaParoleTrovate, client_message.data)){
                    send_message(client_sock, MSG_PUNTI_PAROLA, "0");
                    break;
                }
                // Controllo se parola è in matrice
                else if (!trovaParola(matrice, client_message.data)){
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
                    // Aggiungo la parola alla lista delle parole trovate
                    listaParoleTrovate = aggiungi_parolaTrovata(listaParoleTrovate, client_message.data); // DA SISTEMARE LA LISTA
                    //int puntiparola = strlen(client_message.data);
                    printf("Punti parola \n");
                    // Se la parola contiene "Q" con "u" a seguito, sottraggo di uno i punti
                    if (strstr(client_message.data, "Qu")){
                        //puntiparola--;
                        printf("Sottrai \n");
                    }
                    // Invio i punti della parola
                    char messaggiopuntiparola[90];
                    printf("Punti inviati %ld \n",strlen(client_message.data));
                    sprintf(messaggiopuntiparola, "Con questa parola hai ottenuto %ld punti", strlen(client_message.data));
                    send_message(client_sock, MSG_PUNTI_PAROLA, messaggiopuntiparola);
                    //sprintf("Punteggio inviato \n", messaggiopuntiparola);
                    printf("Punteggio inviato \n");
                    punteggio += strlen(client_message.data); 
                    printf("Il punteggio attuale è %d \n", punteggio); // Nel server visualizzo i punti totali
                    fflush(0);
                }
            } else {
                // Invio il messaggio di errore
                send_message(client_sock, MSG_ERR, "Gioco in pausa!");
            }
            pthread_mutex_unlock(&pausa_gioco_mutex);
            break;

        // domanda: devo modificare per inserire la registrazione qua dentro o posso lasciarla in giocatore?
        case MSG_REGISTRA_UTENTE:
            if (registra_bool == 1) {
                send_message(client_sock, MSG_ERR, "Registrazione già avvenuta, non è possibile registrarsi");
                continue;
            }
            pthread_mutex_lock(&lista_mutex);
            //printf("Debug: Ricevuto MSG_REGISTRA_UTENTE:%s\n",client_message.data);
            registrazione_client(client_sock, client_message.data, &lista);
            utente->username = client_message.data;
            registra_bool = 1;
            pthread_mutex_unlock(&lista_mutex);
            break;

        case MSG_PUNTI_FINALI:
            pthread_mutex_lock(&pausa_gioco_mutex);
            if (pausa_gioco == 1 && classifica != NULL){
                send_message(client_sock, MSG_PUNTI_FINALI, classifica);
                char *temp = calcola_tempo_rimanente(tempo_iniziale, durata_pausa);
                send_message(client_sock, MSG_TEMPO_ATTESA, temp);
            }
            else
            {
                send_message(client_sock, MSG_ERR, "Classifica non disponibile");
            }
            pthread_mutex_unlock(&pausa_gioco_mutex);
            break;

        case MSG_FINE:
            //send_message(client_sock, MSG_FINE, "Disconnessione avvenuta con successo");
            close(client_sock);
            pthread_exit(NULL);
            break;

        case MSG_CANCELLA_UTENTE:
            pthread_mutex_lock(&lista_mutex);
            elimina_giocatore(&lista, client_message.data);
            send_message(client_sock, MSG_OK, "Utente cancellato con successo");
            pthread_mutex_unlock(&lista_mutex);
            break;
        
       // DEVO INSERIRE QUA IL FILE LOG?     
        case MSG_LOGIN_UTENTE:
            pthread_mutex_lock(&lista_mutex);
            if (username_esiste(&lista, client_message.data)) {
                send_message(client_sock, MSG_OK, "Utente già loggato");
                
            } else {
                send_message(client_sock, MSG_ERR, "Username non trovato, per favore registrati prima");
             
            }
            pthread_mutex_unlock(&lista_mutex);
            break;
        
        case MSG_POST_BACHECA:
         printf("\nDebug: Ricevuto MSG_POST_BACHECA\nusername:%s\n",utente->username);

            if (add_message(client_message.data, utente -> username)){
                send_message(client_sock, MSG_OK, "Messaggio postato con successo");
            } else {
                send_message(client_sock, MSG_ERR, "Errore nel postare il messaggio");
            }
        break; 

        case MSG_SHOW_BACHECA:
            pthread_mutex_lock(&mess);
            //char buffer[1024];
            //bacheca_csv(buffer);
            char* buffer = show_bacheca();
            pthread_mutex_unlock(&mess);
            // printf("Debug: Contenuto del buffer:\n%s\n", buffer); // Aggiungi questo messaggio di debug
            send_message(client_sock, MSG_SHOW_BACHECA, buffer);
        break; 
        

        default:
            send_message(client_sock, MSG_ERR, "Comando non valido");
            break;
        }
    }
}

void *scorer() {
    printf("Scorer in esecuzione\n");

    pthread_mutex_lock(&lista_mutex);
    int num_giocatori = lista.count;
    pthread_mutex_unlock(&lista_mutex);

    if (num_giocatori == 0) {
        printf("Nessun giocatore registrato.\n");
        return NULL;
    }

    // Alloca array di puntatori per i giocatori
    giocatore **scorerVector = malloc(num_giocatori * sizeof(giocatore *));
    if (!scorerVector) {
        printf("Errore di allocazione memoria\n");
        return NULL;
    }

    pthread_mutex_lock(&lista_mutex);
    giocatore *current = lista.head;
    for (int i = 0; i < num_giocatori; i++) {
        if (current == NULL) break;
        scorerVector[i] = current;
        current = current->next;
    }
    pthread_mutex_unlock(&lista_mutex);

    // Ordina i giocatori per punteggio
    qsort(scorerVector, num_giocatori, sizeof(giocatore *), compare_score);

    // Costruzione della classifica
    pthread_mutex_lock(&classifica_mutex);
    int max_length = 1024;
    classifica = malloc(max_length);
    if (!classifica) {
        printf("Errore di allocazione memoria per classifica\n");
        free(scorerVector);
        pthread_mutex_unlock(&classifica_mutex);
        return NULL;
    }

    classifica[0] = '\0';
    int offset = 0;

    for (int i = 0; i < num_giocatori; i++) {
        const char *username_safe = scorerVector[i]->username ? scorerVector[i]->username : "Sconosciuto";
        int written = snprintf(classifica + offset, max_length - offset, 
            "%d. %s-%d punti\n", i + 1, username_safe, punteggio);

        if (written < 0 || written >= max_length - offset) {
            printf("Errore nella generazione della classifica\n");
            break;
        }
        offset += written;
    }
    pthread_mutex_unlock(&classifica_mutex);

    printf("Classifica generata:\n%s\n", classifica);

    free(scorerVector);
    return NULL;
}



/* Funzione principale dello scorer
void *scorer() {
    printf("Scorer in esecuzione\n");

    pthread_mutex_lock(&lista_mutex);
    int num_giocatori = lista.count;
    pthread_mutex_unlock(&lista_mutex);

    if (num_giocatori == 0) {
        printf("Nessun giocatore registrato.\n");
        return NULL;
    }

    // Creazione array per ordinamento
    giocatore *scorerVector = malloc(num_giocatori * sizeof(giocatore*));
    if (!scorerVector) {
        printf("Errore di allocazione memoria\n");
        return NULL;
    }

    pthread_mutex_lock(&lista_mutex);
    giocatore *current = lista.head;

    for (int i = 0; current != NULL && i < num_giocatori; i++) {
        scorerVector[i].username = strdup(current->username);
        scorerVector[i].punteggio = current->punteggio;
        scorerVector[i].tid = current->tid;
        current = current->next;
    }
    pthread_mutex_unlock(&lista_mutex);

    qsort(scorerVector, num_giocatori, sizeof(giocatore *), compare_score);

    pthread_mutex_lock(&classifica_mutex);
    classifica = malloc(1024);
    if (!classifica) {
        printf("Errore di allocazione memoria per classifica\n");
        free(scorerVector);
        pthread_mutex_unlock(&classifica_mutex);
        return NULL;
    }
    // DEVO CAMBIARE QUALCOSA
    classifica[0] = '\0';  // Inizializza stringa vuota
    char msg[256];
    for (int i = 0; i < num_giocatori; i++) {
        snprintf(msg, sizeof(msg), "%d. %s %d\n", i + 1, scorerVector[i].username, scorerVector[i].punteggio);
        strcat(classifica, msg);
    }
    pthread_mutex_unlock(&classifica_mutex);
    printf("Classifica generata:\n%s\n", classifica);
    

    for (int i = 0; i < num_giocatori; i++) {
        free(scorerVector[i].username);
    }
    free(scorerVector);

    // Invio della classifica a tutti i giocatori
    pthread_mutex_lock(&lista_mutex);
    current = lista.head;
    pthread_mutex_unlock(&lista_mutex);

    time_t tempo_iniziale = time(NULL);
    int durata_pausa = 30;

    while (current != NULL) {
        sendClassifica(&lista, current->tid, classifica, tempo_iniziale, durata_pausa);
        current = current->next;
    }

    pthread_mutex_lock(&classifica_mutex);
    free(classifica);
    pthread_mutex_unlock(&classifica_mutex);

    printf("Classifica inviata a tutti i giocatori.\n");
    return NULL;
}*/

void *game(void *arg){
    int round = 0;
    while(1){
        if(lista.count == 0){
            printf("Nessun giocatore registrato, attesa...\n");
            //ATTESA FINO A QUANDO NON SI REGISTRA UN NUOVO GIOCATORE
            while(lista.count == 0){
                time(&tempo_iniziale);
            }
        }
        //REGISTRAZIONE DEI SEGNALI
        //signal(SIGUSR2, alarm_handler);
        
        //INIZIA LA PAUSA
        time(&tempo_iniziale);
        alarm(durata_pausa);
        printf("-----------------------------------------------------------------------\n");
        printf("Il round è terminato, inizierà tra: %d secondi\n", durata_pausa);

        //SE IL ROUND NON è STATO PREPARATO ALLORA LO PREPARA
        if(round == 0){
            //pthread_mutex_lock(&matrix_mutex);
            FILE *file = fopen("../Matrici.txt", "rb"); //
             if (file == NULL)
            {
                perror("Errore nell'apertura del file");
                return NULL;  
            } 
            //cella **matrice = generateMatrix();
            Carica_MatricedaFile(file, matrice);        // Carica i dati della matrice dal file
            fclose(file);
            //pthread_mutex_unlock(&matrix_mutex);
            round = 1;
        }

        while(pausa_gioco){
            //attesa
        }
        //FINE ATTESA

        //SE ALLA FINE DELLA PAUSA NON CI SONO GIOCATORI REGISTRATI, SI RIPETE IL CICLO DA CAPO E SI ATTENDONO NUOVI GIOCATPORI, MANTENENDO IL ROUND GIÀ PREPARATO
        if(lista.count == 0){
            pausa_gioco = 1;
            continue;
        }

        //INIZIA IL GIOCO
        time(&tempo_iniziale);
        struct tm* timeinfo = localtime(&tempo_iniziale);
        char* orario_completo = asctime(timeinfo);
        char orario[9];
        snprintf(orario, sizeof(orario), "%.8s", orario_completo + 11);
        alarm(durata_partita);
        printf("-----------------------------------------------------------------------\n");
        printf("La partita è iniziata alle %s con %d giocatori\n", orario_completo, lista.count);
        
        //INVIO DELLA MATRICE E DEL TEMPO RIMANENTE AI GIOCATORI REGISTRATI E QUINDI PARTECIPANTI AL GIOCO
        pthread_mutex_lock(&lista_mutex);
        giocatore* current = lista.head;
        while (current != NULL){
            invio_matrice(current->client_fd, matrice);
            char *temp = calcola_tempo_rimanente(tempo_iniziale, durata_partita);
            send_message(current->client_fd, MSG_TEMPO_PARTITA, temp);
            printf("Inviato a giocatore %d\n", current->client_fd);
            current = current->next;
        }
        pthread_mutex_unlock(&lista_mutex);
        
        while(!pausa_gioco){
            //attesa
        }
        //FINE GIOCO
        //alarm(durata_partita);
        //printf("Il turno è di: %d\n", turno);
        //NOTIFICA CHE IL ROUND PREPARATO È STATO UTILIZZATO E QUINDI BISOGNA PREPARARNE UNO NUOVO
        round = 0;
    }
}

void alarm_handler(int sig){
    switch (pausa_gioco)
    {//0 giocando, 1 pausa
    case 1://è finita la pausa
        //FINISCE LA PAUSA
        //ISTANZIO ALLARME
        alarm(durata_partita);
        //MANDO LA MATRICE
        //CAMBIO STATO DI GIOCO
        pausa_gioco = 0;//metto gioco in gioco
        printf("allarme partita mandato\nIl gioco è in corso\n");
        return;
    
    case 0://è finita la partita
        //FINISCE LA PARTITA
        //RESETTO GIOCO 
        turno = 0;
        game_started = 0;pausa_gioco = 1;//metto gioco in pausa
        printf("La partita è finita, inizierà tra: %d secondi\n", durata_pausa);
        alarm(durata_pausa);
        //CHIAMO SCORER
        //LA ALARM SI MANDA DA SOLA nel corpo di prova
        //DICO A TUTTI I THREAD DI MANDARE PUNTEGGIO ED USERNAME SULLA CODA PROD-CONS -> invia_sig
        if (lista.count > 0){
            invia_SIG(&lista, SIGUSR2, lista_mutex);
            printf("Invio segnale SIGUSR2\n");
            //int retvalue = pthread_create(&scorer_tid, NULL, scorer, NULL);
            //if (retvalue != 0){
            //    perror("Errore nella pthread_create dello scorer");
            //}
        } else {
            printf("Nessun giocatore registrato, niente SIGUSR2 \n");
        }
        printf("Il gioco è terminato\n");
        break;
    }
}

int main(int argc, char *argv[]){
    Dizionario = create_node();
    Load_Dictionary(Dizionario, DIZIONARIO);
    //insert_Trie(Dizionario, "CIAO");
 
    //printf("ciao %d\n", search_Trie("CIAO", Dizionario));
    //Print_Trie();
    int server_sock;
    struct sockaddr_in server_addr;
    // char message [128];
    // Segnale per il SERVER_SHUTDOWN
    signal(SIGINT, sigint_handler);
    signal(SIGUSR2, sigusr2_classifica_handler);
    // Definizione dei segnali
    signal(SIGALRM, alarm_handler);

    if (argc < 3){
        printf("Errore: Numero errato di parametri passiti, aspettati : 3, ricevuti: %d \n" , argc);
        return 0;
    }
    // Creazione del socket
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    // Se la connessione fallisce, invio messaggio di errore
    if (server_sock < 0){
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
    // BIND() assegna l'indirizzo specificato nella struttura server_addr al socket server_sock
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
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
    pthread_t partita;
    int retvalue;
    matrice = generateMatrix();
    SYST(retvalue, pthread_create(&partita, NULL, game, NULL), "nella creazione del thread di gioco");
    while (1)
    {
        // Accetta la connessione
        int client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_len);
        printf("Connessione accettata\n");
        // THREAD
        // Dichiara thread
        pthread_t thread_id;

        // Creazione del thread a cui passo il fd associato al socket del client
        if (pthread_create(&thread_id, NULL, thread_func, &client_sock) != 0){
            // Se fallisce la creazione del thread, stampa un messaggio di errore e termina
            perror("Failed to create thread");
            return 1;
        }
    }
}