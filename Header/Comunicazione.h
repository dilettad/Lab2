//definizione tipi di messaggio
#define MSG_OK 'K'
#define MSG_ERR 'E'
#define MSG_REGISTRA_UTENTE 'R'
#define MSG_MATRICE 'M'
#define MSG_TEMPO_PARTITA 'T'
#define MSG_TEMPO_ATTESA 'A'
#define MSG_PAROLA 'W'
#define MSG_PUNTI_FINALI 'F'
#define MSG_PUNTI_PAROLA 'P'
#define MSG_SERVER_SHUTDONW 'B'
#define MSG_POST_BACHECA 'H'
#define MSG_SHOW_BACHECA 'S'
#define MSG_FINE 'Q'
#define MSG_CANCELLA_UTENTE 'D' // DA FARE 
#define MSG_LOGIN_UTENTE 'L'  // DA FARE
#define MAX_LENGTH_USERNAME 10 //Numero massimo di lunghezza dell'username
#define MAX_CLIENTS 32 // Numero massimo di giocatori


//Definizione di una struttura richiesta e risposta
typedef struct {
    char type; 
    unsigned int length; 
    char* data; 
} message;

typedef struct Fifo Fifo;


//Funzione per inviare un messaggio
void send_message(int client_socket, char type, char* data);
//Funzione per ricevere un messaggio
message receive_message(int client_socket);

//Funzione per registrazione del cliente
void registrazione_client(int client_fd, char* username);

//Funzione per aggiungere client alla lista
void add_client(Fifo* lista, int client_fd, char* username);

//Controllo caratteri dell'username: non deve contenere caratteri ASCII
int controlla_caratteri(const char* username);

// Funzione per stampare la lista dei client
void stampa_lista_clienti();

// Funzione per liberare la memoria della lista
void libera_lista_clienti();

