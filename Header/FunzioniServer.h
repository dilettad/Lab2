
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
    pthread_mutex_t lista_mutex;
}listaGiocatori;



/* Calcola tempo rimanente
void calcola_tempo_rimanente(time_t tempo_iniziale, int durata) {
    time_t tempo_attuale = time(NULL);
    double tempo_trascorso = difftime(tempo_attuale, tempo_iniziale);
    int tempo_rimanente_secondi = durata - (int)tempo_trascorso;

    if (tempo_rimanente_secondi < 0) {
        printf("Il gioco è già terminato\n");
    } else {
        printf("Il tempo rimanente è: %d secondi\n", tempo_rimanente_secondi);
    }
}
*/

