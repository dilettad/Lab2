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
#define MSG_SERVER_SHUTDOWN 'B' 
#define MSG_POST_BACHECA 'H'
#define MSG_SHOW_BACHECA 'S'
#define MSG_FINE 'Q' 
#define MSG_CANCELLA_UTENTE 'D' 
#define MSG_LOGIN_UTENTE 'L'  

#define MAX_LENGTH_USERNAME 10 //Numero massimo di lunghezza dell'username
#define MAX_CLIENTS 32 // Numero massimo di giocatori

//Definizione di una struttura richiesta e risposta
typedef struct {
    char type; 
    unsigned int length; 
    char* data; 
    char username[50];
} message;

typedef struct Fifo Fifo;

//Funzione per inviare un messaggio
void send_message(int client_socket, char type, char* data);
//Funzione per ricevere un messaggio
message receive_message(int client_socket);



