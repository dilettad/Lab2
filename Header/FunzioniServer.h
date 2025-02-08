
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>

//#include "../Header/Comunicazione.h"

//typedef struct Fifo Fifo;

char*  calcola_tempo_rimanente(time_t tempo_iniziale, int durata_partita);

//void sendClassifica(listaGiocatori* lista, pthread_t tid, pthread_mutex_t lista_mutex, char* classifica, time_t tempo_iniziale, int durata_pausa);
void sendClassifica(listaGiocatori* lista, pthread_t tid, char* classifica, time_t tempo_iniziale, int durata_pausa);

int prendi_punteggi(listaGiocatori* lista, pthread_t tid);
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

