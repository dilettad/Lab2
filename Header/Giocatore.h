// #include "../Header/Lista.h"

//Funzione per aggiungere client alla lista
void add_client(listaGiocatori *lista, int client_fd, char *username);

// Funzione per registrazione del cliente
void registrazione_client(int client_fd, char *username, listaGiocatori *lista);

//Funzione per controllare se username è già esistente
int username_esiste(listaGiocatori* lista, char *username);


// Funzione login utente
int login_utente(int client_fd, listaGiocatori *lista, char *username);

// Controllo caratteri dell'username: non deve contenere caratteri ASCII
int controlla_caratteri(const char *username);

//Funzione per eliminare il giocatore dalla lista dei giocatori
void elimina_giocatore(listaGiocatori *lista, char *username);

//Funzione per eliminare un thread dalla lista dei clients dopo un periodo di inattività
void elimina_thread(Fifo *clients, pthread_t thread_id, pthread_mutex_t *clients_mutex);