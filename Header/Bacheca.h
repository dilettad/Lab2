#include <pthread.h>
#define MAX_MESSAGES 8 //Numero massimo dei messaggi
#define MAX_LENGTH_MESSAGE 128 //Numero massimo di lunghezza del messaggio

extern pthread_mutex_t mess; 

//Struttura messaggio
typedef struct {
    char username [32];
    char text[MAX_LENGTH_MESSAGE];
   // int   count [MAX_MESSAGES];
} Message; 

//Funzione per aggiungere messaggi nella bacheca
int add_message(char* text, char* username);

//Funzione per mostrare i messaggi all'interno della bacheca
char* show_bacheca();

//Funzione per postare i messaggi sulla bacheca facendone una copia
Message *post_messaggi(int message_count);


