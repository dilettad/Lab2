// #include "../Header/Lista.h"
void add_client(Fifo *lista, int client_fd, char *username);

// Funzione registrazione cliente
void registrazione_client(int client_fd, char *username, listaGiocatori *lista);

// Funzione controlla_caratteri
int controlla_caratteri(const char *username);

// Funzione stampa lista clienti
void stampa_lista_clienti(Fifo *lista);

void elimina_giocatore(listaGiocatori *lista, char *username, pthread_mutex_t lista_mutex);