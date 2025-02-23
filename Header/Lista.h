
// MSG_REGISTRA UTENTE con la gestione del MSG_ERR e MSG_OK

// Definizione del tipo di dati per struttura utente
#include <pthread.h>
#include <netinet/in.h>
#include "Matrice.h"
#define MSG_SERVER_SHUTDOWN 'B'

struct Client;

//Definizione della struttura client
typedef struct Client{
    char *username;                 // Nome utente 
    int fd;                         // File descriptor
    int score;                      // Punteggio utente
    int socket;                     // Socket 
    int isPlayer;                   // Stato (attivo o non)
    struct Client *next;            // Puntatore al prossimo utente (per la lista)
    struct sockaddr_in address;     // Indirizzo del client 
    pthread_t thread_id;            // ID del thread
    time_t last_activity;           // Ultima attività dell'utente 
    paroleTrovate *paroleTrovate;   // Puntatore alle parole trovate dal client
} Client;

//Definizione della struttura Fifo 
typedef struct Fifo{
    Client *head;                   // Puntatore alla testa 
    Client *tail;                   // Puntatore alla coda
    int size;                       // Contatore
} Fifo;

//Definizione delle struttura giocatore
typedef struct giocatore{
    char *username;                 // Username
    pthread_t tid;                  // ID del thread associato
    int punteggio;                  // Punteggio giocatore
    int count;                      // Contatore
    int active;                     // Stato giocatore (attivo o no)
    struct giocatore *next;         // Puntatore al prossimo giocatore
} giocatore;

//Definizione della struttura lista giocatori
typedef struct{
    giocatore *head;                //Puntatore alla testa
    giocatore *tail;                //Puntatore alla coda
    int count;                      //Contatore
} listaGiocatori;

//Funzione per aggiungere un client alla coda
void push(Fifo *lista, Client *new_client);

//Funzione per elimina un client dalla Fifo
void deleteClient(Fifo *lista,pthread_t tid);

//Funzione per creare una nuova Fifo
Fifo *create();


