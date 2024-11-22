
#define MAX_LENGTH_USERNAME 10 //Numero massimo di lunghezza dell'username
#define MAX_CLIENTS 32 


//GIOCATORE 
//Funzione per aggiungere client alla lista
void add_client(Fifo* lista, int client_fd, char* username);
  
//Funzione per registrazione del cliente
void registrazione_client(int client_fd, char* username);

// //Controllo caratteri dell'username: non deve contenere caratteri ASCII
int controlla_caratteri(const char* username);

// //Funzione per stampare la lista dei client
void stampa_lista_clienti();

// // Funzione per liberare la memoria della lista
void libera_lista_clienti();


