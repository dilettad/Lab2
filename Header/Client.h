#include "../Header/macro.h"
#include "../Header/Comunicazione.h"
#include "../Header/Lista.h"

//Funzione per aggiungere client alla lista
void add_client(Client** head, int client_fd, char*username);

//Funzione per registrazione del cliente
void registrazione_client(int client_fd,char* username);

//Funzione per controllare i caratteri dell'username
int controlla_caratteri(const char* username);

//Funzione per stampare la lista dei clienti
void stampa_lista_clienti();

// Funzione per liberare la memoria della lista
void libera_lista_clienti();
