//MSG_REGISTRA UTENTE con la gestione del MSG_ERR e MSG_OK

//Definizione del tipo di dati per struttura utente
typedef struct Client {
    char* username; //Nome utente
    int fd; // File descriptor del client
    int score; //Punteggio utente
    struct Client* next; //Puntatore al prossimo utente
} Client; 

typedef struct Fifo {
    Client* head;
    Client* tail;
    int size;
}Fifo;

void push (Fifo * lista, Client *new_client);
Client * pop (Fifo * list) ;
int seek (Fifo * list, char* username); // 1 se trova un cliente con quel username 0 altrimenti
Fifo* create ();

