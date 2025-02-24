
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>

typedef struct{
    char *matrix_file;
    float durata_partita;
    long seed;
    char *file_dizionario;
} Parametri;

//Funzione per calcolare il tempo rimanente della partita
char*  calcola_tempo_rimanente(time_t tempo_iniziale, int durata_partita);

//Funzione per inviare la classifica ai giocatori
void sendClassifica(listaGiocatori* lista, pthread_t tid, char* classifica, time_t tempo_iniziale, int durata_pausa);

//Funzione per confrontare i punteggi dei giocatori -> qsort
int compare_score(const void *a, const void *b);

//Funzione per gestire i punteggio, cercandoli nella lista dei giocatori tramite tid
int prendi_punteggi(listaGiocatori* lista, pthread_t tid);

//Funzione per caricare in memoria il dizionario
void Load_Dictionary(Trie *Dictionary, char *path_to_dict);

//Funzione per identificare il giocatore
giocatore* trova_giocatore(listaGiocatori* lista, pthread_t tid);

//Funzione per resettare i punteggi 
void reset_punteggi();
