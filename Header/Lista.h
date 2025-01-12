
// MSG_REGISTRA UTENTE con la gestione del MSG_ERR e MSG_OK

// Definizione del tipo di dati per struttura utente
#include <pthread.h>

struct Client;

typedef struct Client
{
    char *username;      // Nome utente
    int fd;              // File descriptor del client
    int score;           // Punteggio utente
    struct Client *next; // Puntatore al prossimo utente (per la lista)
    int active;          // Se attivo(1) o no (0)
    time_t last_activity; // Ultima attività dell'utente
} Client;

typedef struct Fifo
{
    Client *head;
    Client *tail;
    int size;
} Fifo;

// Struttura del giocatore registrato
typedef struct giocatore
{
    char *username;
    pthread_t tid;
    int client_fd;
    int punteggio;
    struct giocatore *next;
} giocatore;

// Lista giocatori registrati
typedef struct
{
    giocatore *head;
    giocatore *tail;
    int count;
    pthread_mutex_t lista_mutex;
    pthread_cond_t lista_cond;
    pthread_mutex_t lock;
} listaGiocatori;

void push(Fifo *lista, Client *new_client);
Client *pop(Fifo *list);
int seek(Fifo *list, char *username); // 1 se trova un cliente con quel username 0 altrimenti
Fifo *create();
void aggiorna_punteggio(listaGiocatori *lista, char *username, int punteggio);
void distruggi_lista(listaGiocatori *lista);
