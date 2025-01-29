// #include "../Header/Lista.h"
void add_client(listaGiocatori *lista, int client_fd, char *username);

// Funzione registrazione cliente
void registrazione_client(int client_fd, char *username, listaGiocatori *lista);

// Funzione controlla_caratteri
int controlla_caratteri(const char *username);

int username_esiste(listaGiocatori* lista, char *username);

// Funzione stampa lista clienti
void stampa_lista_clienti(Fifo *lista);

void elimina_giocatore(listaGiocatori *lista, char *username, pthread_mutex_t lista_mutex);

//void elimina_thread(Fifo * lista, int fd, pthread_mutex_t *clients_mutex);

void elimina_thread(Fifo *clients, pthread_t thread_id, pthread_mutex_t *clients_mutex);
giocatore* RecuperaUtente(listaGiocatori* newLista, char* username);