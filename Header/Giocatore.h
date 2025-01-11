//#include "../Header/Lista.h"
void add_client(Fifo* lista, int client_fd, char* username);

// Funzione registrazione cliente
void registrazione_client(int client_fd, char* username, Fifo* lista);

// Funzione controlla_caratteri
int controlla_caratteri(const char* username);

// Funzione stampa lista clienti
void stampa_lista_clienti(Fifo* lista);

void elimina_giocatore(Fifo* lista, int client_fd);