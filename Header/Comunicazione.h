//Definizione del messaggio
typedef struct{
    char type;
    unsigned int length;
    char* data;
} msg;

//invio messaggio
void send_message(int client_socket, char type, char* data);
//ricezione messaggio sul socket
message receive_message(int client_socket);

//definizione tipi di messaggio
#define MSG_OK "K"
#define MSG_ERR "E"
#define MSG_REGISTRA_UTENTE "R"
#define MSG_MATRICE "M"
#define MSG_TEMPO_PARTITA "T"
#define MSG_TEMPO_ATTESA "A"
#define MSG_PAROLA "W"
#define MSG_PUNTI_FINALI "F"
#define MSG_PUNTI_PAROLA "P"
#define MSG_SERVER_SHUTDONW "B"
#define MSG_POST_BACHECA "H"
#define MSG_SHOW_BACHECA "S"