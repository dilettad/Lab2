#include <pthread.h>
#define MAX_MESSAGES 8 //Numero massimo dei messaggi
#define MAX_LENGTH_MESSAGE 128 //Numero massimo di lunghezza del messaggio

pthread_mutex_t mess = PTHREAD_MUTEX_INITIALIZER; 

//Struttura messaggio
typedef struct {
    char username [32];
    char text[MAX_LENGTH_MESSAGE];
   // int   count [MAX_MESSAGES];
} Message; 

//Funzione per aggiungere messaggi in bacheca
int add_message(char* text, char* username);
char* show_bacheca();

//Funzione per post messaggi sulla bacheca
Message *post_messaggi(int message_count);

void bacheca_csv(char *filename);

void libera_messaggi(Message * messaggi, int num_messaggi);