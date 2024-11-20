#define MAX_CLIENTS 32 
#define MAX_LENGTH_USERNAME 10 //Numero massimo di lunghezza dell'username

//MSG_REGISTRA UTENTE con la gestione del MSG_ERR e MSG_OK
//Definizione del tipo di dati per struttura utente
typedef struct Client {
    char username; //Nome utente
    int score; //Punteggio utente
    struct Client* next; //Puntatore al prossimo utente
} Client; 


//Funzione per aggiungere client alla lista
void add_client(Client** head, char*username);

//Funzione per registrazione del cliente
void registrazione_client(int client_fd,char* username);

//Funzione per controllare i caratteri dell'username
int controlla_caratteri(const char* username);

//Funzione per stampare la lista dei clienti
void stampa_lista_clienti();