#define MAX_MESSAGES 8 //Numero massimo dei messaggi
#define MAX_LENGTH_MESSAGE 128 //Numero massimo di lunghezza del messaggio

//Struttura messaggio
typedef struct {
    char* username [32];
    char* text[MAX_LENGTH_MESSAGE];
} Message; 

//Funzione per aggiungere messaggi in bacheca
void add_message(char* text, char* username);

//Funzione per post messaggi sulla bacheca
Message *post_messaggi(int message_count);