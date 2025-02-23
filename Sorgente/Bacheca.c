// 13° Punto del server: messaggi
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "../Header/Bacheca.h"
#include "../Header/Comunicazione.h"

//Array per memorizzare gli ultimi 8 messaggi
Message messages [MAX_MESSAGES];
int message_count = 0; //Inizializzo il contatore dei messaggi a 0
//Inizializzo thread mutex 

pthread_mutex_t mess = PTHREAD_MUTEX_INITIALIZER;

// Funzione per aggiungere messaggi in bacheca
int add_message(char* text, char* username){
    pthread_mutex_lock(&mess);
    // Controllo posso inserire il messaggio in quanto < 8 
    if (message_count < MAX_MESSAGES){
        strncpy(messages[message_count].text, text, MAX_LENGTH_MESSAGE - 1);
        messages[message_count].text[MAX_LENGTH_MESSAGE - 1] = '\0';
        strncpy(messages[message_count].username, username, MAX_LENGTH_USERNAME  - 1);
        messages[message_count].username[MAX_LENGTH_USERNAME - 1] = '\0';
        message_count++;
        pthread_mutex_unlock(&mess);
        return 1; // Successo
    } else {         
        printf("Bacheca piena\n");
        // Sposto i messaggi in modo da lasciare l'ultimo posto in memoria libero
        for (int i = 1; i < MAX_MESSAGES; i++){
            messages[i-1] = messages[i];
        }
        // Aggiungo il nuovo messaggio all'ultimo posto
        strncpy(messages[MAX_MESSAGES - 1].text, text, MAX_LENGTH_MESSAGE - 1);
        messages[MAX_MESSAGES - 1].text[MAX_LENGTH_MESSAGE - 1] = '\0';
        strncpy(messages[MAX_MESSAGES - 1].username, username, MAX_LENGTH_USERNAME - 1);
        messages[MAX_MESSAGES - 1].username[MAX_LENGTH_USERNAME - 1] = '\0';
        pthread_mutex_unlock(&mess);
        return 1; // Successo
    }
    pthread_mutex_unlock(&mess);    
    return 0; // Fallimento
}


char* show_bacheca(){
    //pthread_mutex_lock(&mess);

    // Alloca un buffer per la risposta
    char *buffer = malloc(MAX_MESSAGES * 160); // 128 + 32 username + extra
    if (buffer == NULL) {
        //pthread_mutex_unlock(&mess);
        return NULL;
    }

    buffer[0] = '\0';  // Inizializza la stringa

    for (int i = 0; i < message_count; i++) {
        strcat(buffer, messages[i].username);
        strcat(buffer, ": ");
        strcat(buffer, messages[i].text);
        strcat(buffer, "\n");
    }

    //pthread_mutex_unlock(&mess);
    return buffer;
} 


Message *post_messaggi(int message_count) {
    pthread_mutex_lock(&mess);
    Message* read_message = malloc(message_count * sizeof(Message));

    // Controllo se già letti
    if (read_message == NULL) {
        pthread_mutex_unlock(&mess);
        return NULL;
    }

    // Inserisco i messaggi 
    for (int i = 0; i < message_count; i++) {
        strncpy(read_message[i].text, messages[i].text, MAX_LENGTH_MESSAGE - 1);
        read_message[i].text[MAX_LENGTH_MESSAGE - 1] = '\0';
        strncpy(read_message[i].username, messages[i].username, 32 - 1);
        read_message[i].username[32 - 1] = '\0';

    }
    pthread_mutex_unlock(&mess);
    return read_message;
}
