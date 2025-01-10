// 13° Punto del server: messaggi
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "../Header/Bacheca.h"

//Array per memorizzare gli ultimi 8 messaggi
Message messages [MAX_MESSAGES];
int message_count = 0; //Inizializzo il contatore dei messaggi a 0
//Inizializzo thread mutex
pthread_mutex_t mess = PTHREAD_MUTEX_INITIALIZER;
 
//Funzione per aggiungere messaggi in bacheca
void add_message(char* text, char* username){
    pthread_mutex_lock (&mess);

    //Controllo posso inserire il messaggio in quanto < 8 
    if (message_count < MAX_MESSAGES){
        //Aggiungo il messaggio copiandolo
        strcpy(messages[message_count].text, text);
        strcpy(messages[message_count].username, username);
        message_count++;
        } else  {         
            printf("Bacheca piena\n");
            //Libero la memoria del messaggio più vecchio
            free(messages[0].text);
            free(messages[0].username);
            //Sposto i messaggi in modo da lasciare l'ultimo posto in memoria libero
            for (int i = 1; i < MAX_MESSAGES; i++){
                messages[i-1] = messages[i];
            }
            // Aggiungo il nuovo messaggio all'ultimo posto
            strcpy(messages[MAX_MESSAGES - 1].text, text);
            strcpy(messages[MAX_MESSAGES - 1].username, username);          
        }
    pthread_mutex_unlock(&mess);    
}
//TESTATA: FUNZIONA


Message *post_messaggi(int message_count) {
    pthread_mutex_lock(&mess);

    // Alloco memoria per l'array di messaggi da restituire
    // Message* read_message = (Message*)malloc(message_count * sizeof(Message));
    Message *read_message = malloc(message_count * sizeof(Message));

    // Controllo se già letti
    if (read_message == NULL) {
        pthread_mutex_unlock(&mess);
        return NULL;
    }

    // Inserisco i messaggi 
    for (int i = 0; i < message_count; i++) {
        read_message[i].text = malloc(strlen(messages[i].text) + 1);
        if (read_message[i].text == NULL) {
            // Gestire l'errore di allocazione
            for (int j = 0; j < i; j++) {
                free(read_message[j].text);
                free(read_message[j].username);
            }
            free(read_message);
            pthread_mutex_unlock(&mess);
            return NULL;
        }
        strcpy(read_message[i].text, messages[i].text);
        
        read_message[i].username = malloc(strlen(messages[i].username) + 1);
        if (read_message[i].username == NULL) {
            // Gestire l'errore di allocazione
            free(read_message[i].text);
            for (int j = 0; j < i; j++) {
                free(read_message[j].text);
                free(read_message[j].username);
            }
            free(read_message);
            pthread_mutex_unlock(&mess);
            return NULL;
        }
        strcpy(read_message[i].username, messages[i].username);
    }
    pthread_mutex_unlock(&mess);
    return read_message;
}

// Funzione per generare il contenuto della bacheca in formato CSV
void bacheca_csv(char *filename){
    //Apre il file specificato da 'filename' in modalità scrittura
    FILE *file = fopen(filename, "w");
    //Se file è null, errore nell'apertura
    if (file == NULL) {
        printf("Errore nell'apertura del file\n");
        return;
    }
    //Scrive l'intestazione nel file CSV, scrivendo username o text
    fprintf(file, "username,text\n");
    // Iterare ciclo e scrivo nel file i dettagli di ogni messaggio
    for (int i = 0; i < message_count; i++) {
        fprintf(file, "%s,%s\n", messages[i].username, messages[i].text);
    }

    // Chiusura file
    fclose(file); 
}
// TESTATA: NI FUNZIONA
