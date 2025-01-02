
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>


//Struttura del giocatore registrato
typedef struct giocatore {
    char* username;
    pthread_t tid;
    int client_fd;
    int punteggio;
    struct giocatore* next;
}giocatore;

//Lista giocatori registrati
typedef struct{
    giocatore* head;
    giocatore* tail;
    int count;
}listaGiocatori;

int pausa_gioco = 0; //Gioco
// La partita dura 5 minuti quindi 300s
int durata_partita = 300;
//La pausa della partita dura 1.5 minuti
int durata_pausa = 90;


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

/* Funzioni get
// void get_username(listaGiocatori* lista, pthread_t tid, int num_giocatori){
//     listaGiocatori* lista_cpy = lista;
//     for (int i = 0; i < num_giocatori; i++) {
//         if (pthread_equal(lista->head->tid, tid)){
//             return lista->head.username;
//         }else{
//             lista_cpy->head = lista_cpy->head->next;
//         }
//     }
//     return NULL; // ID non trovato
// }

ATTRAVERSO IL TID RISALE AL PUNTEGGIO DEL GIOCATORE
int get_punteggio(listaGiocatori* lista, pthread_t tid) {
    listaGiocatori* lista_cpy = lista;
    while(lista_cpy->head != NULL){
        if (pthread_equal(lista_cpy->head->tid, tid)) {
            return lista_cpy->head->punteggio;
        }
        lista_cpy->head = lista_cpy->head->next;
    }
    return -1; // Thread ID non trovato (o punteggio non valido)
}
*/

//Funzione di invio classifica ai giocatori
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

pthread_mutex_t lista_mutex = PTHREAD_MUTEX_INITIALIZER; 

/* Funzione di invio segnali ai tutti i giocatori della lista
void invia_SIG(listaGiocatori* lista, int SIG, pthread_mutex_t* lista_mutex){
    pthread_mutex_lock(&lista_mutex);
    giocatore* current = lista->head;
    while(current != NULL){
       pthread_kill(current->tid, SIG);
        current = current->next;
        }
        pthread_mutex_unlock(lista_mutex);
}
*/
