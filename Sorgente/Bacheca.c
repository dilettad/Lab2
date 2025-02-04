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

 
/*Funzione per aggiungere messaggi in bacheca
int add_message(char* text, char* username){
    pthread_mutex_lock (&mess);

    //Controllo posso inserire il messaggio in quanto < 8 
    if (message_count < MAX_MESSAGES){
        //Aggiungo il messaggio copiandolo
        strcpy(messages[message_count].text, text);
        //messages[message_count].text = '\0';
        strcpy(messages[message_count].username, username);
        messages[message_count].text [MAX_LENGTH_MESSAGE - 1] = '\0';
        message_count++;
        pthread_mutex_unlock(&mess);
        return 1;
    } else {         
            printf("Bacheca piena\n");
            //Libero la memoria del messaggio più vecchio
            //free(messages[0].text);
            //free(messages[0].username);
            //Sposto i messaggi in modo da lasciare l'ultimo posto in memoria libero
            for (int i = 1; i < MAX_MESSAGES; i++){
                strcpy(messages[i - 1].text, messages[i].text);
                strcpy(messages[i - 1].username, messages[i].username);
               // messages[i-1] = messages[i];
            }
            // Aggiungo il nuovo messaggio all'ultimo posto
            strncpy(messages[MAX_MESSAGES - 1].text);
            //messages[MAX_MESSAGES - 1].text[MAX_LENGTH_MESSAGE] = '\0';
            strncpy(messages[MAX_MESSAGES - 1].username);
            messages[MAX_MESSAGES - 1].text[MAX_LENGTH_MESSAGE - 1] = '\0';
            //pthread_mutex_unlock(&mess);          
            //return 1;
        }
    pthread_mutex_unlock(&mess);    
    return 1;
}
*/
/*
// Funzione per aggiungere messaggi in bacheca
int add_message(char* text, char* username){
    pthread_mutex_lock(&mess);

    // Controllo posso inserire il messaggio in quanto < 8 
    if (message_count < MAX_MESSAGES){
        // Aggiungo il messaggio copiandolo
        messages[message_count].text = malloc(strlen(text) + 1);
        messages[message_count].username = malloc(strlen(username) + 1);
        if (messages[message_count].text == NULL || messages[message_count].username == NULL) {
            pthread_mutex_unlock(&mess);
            return 0; // Fallimento
        }
        strcpy(messages[message_count].text, text);
        strcpy(messages[message_count].username, username);
        message_count++;
        pthread_mutex_unlock(&mess);
        return 1; // Successo
    } else {         
        printf("Bacheca piena\n");
        // Libero la memoria del messaggio più vecchio
        free(messages[0].text);
        free(messages[0].username);
        // Sposto i messaggi in modo da lasciare l'ultimo posto in memoria libero
        for (int i = 1; i < MAX_MESSAGES; i++){
            messages[i-1] = messages[i];
        }
        // Aggiungo il nuovo messaggio all'ultimo posto
        messages[MAX_MESSAGES - 1].text = malloc(strlen(text) + 1);
        messages[MAX_MESSAGES - 1].username = malloc(strlen(username) + 1);
        if (messages[MAX_MESSAGES - 1].text == NULL || messages[MAX_MESSAGES - 1].username == NULL) {
            pthread_mutex_unlock(&mess);
            return 0; // Fallimento
        }
        strcpy(messages[MAX_MESSAGES - 1].text, text);
        strcpy(messages[MAX_MESSAGES - 1].username, username);
        pthread_mutex_unlock(&mess);
        return 1; // Successo
    }
    pthread_mutex_unlock(&mess);    
    return 0; // Fallimento
}
*/

// Funzione per aggiungere messaggi in bacheca
int add_message(char* text, char* username){
    pthread_mutex_lock(&mess);

    // Controllo posso inserire il messaggio in quanto < 8 
    if (message_count < MAX_MESSAGES){
        // Aggiungo il messaggio copiandolo
        strncpy(messages[message_count].text, text, MAX_LENGTH_MESSAGE - 1);
        messages[message_count].text[MAX_LENGTH_MESSAGE - 1] = '\0';
        strncpy(messages[message_count].username, username, 32  - 1);
        messages[message_count].username[32 - 1] = '\0';
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
        strncpy(messages[MAX_MESSAGES - 1].username, username, 32 - 1);
        messages[MAX_MESSAGES - 1].username[32 - 1] = '\0';
        pthread_mutex_unlock(&mess);
        return 1; // Successo
    }
    pthread_mutex_unlock(&mess);    
    return 0; // Fallimento
}

char* show_bacheca(){
    pthread_mutex_lock(&mess);

    // Alloca un buffer per la risposta
    char *buffer = malloc(MAX_MESSAGES * 160); // 128 + 32 username + extra
    if (buffer == NULL) {
        pthread_mutex_unlock(&mess);
        return NULL;
    }

    buffer[0] = '\0';  // Inizializza la stringa

    for (int i = 0; i < message_count; i++) {
        strcat(buffer, messages[i].username);
        strcat(buffer, ": ");
        strcat(buffer, messages[i].text);
        strcat(buffer, "\n");
    }

    pthread_mutex_unlock(&mess);
    return buffer;
}

Message *post_messaggi(int message_count) {
    pthread_mutex_lock(&mess);

    // Alloco memoria per l'array di messaggi da restituire
    // Message* read_message = (Message*)malloc(message_count * sizeof(Message));
    Message* read_message = malloc(message_count * sizeof(Message));

    // Controllo se già letti
    if (read_message == NULL) {
        pthread_mutex_unlock(&mess);
        return NULL;
    }

    // Inserisco i messaggi 
    for (int i = 0; i < message_count; i++) {
        // read_message[i].text = malloc(strlen(messages[i].text) + 1);
        // read_message[i].username = malloc(strlen(messages[i].username) + 1);
        /*
        if (read_message[i].text == NULL || read_message[i].username == NULL) {
            // Gestire l'errore di allocazione
            for (int j = 0; j < i; j++) {
                free(read_message[j].text);
                free(read_message[j].username);
            }
            free(read_message);
            pthread_mutex_unlock(&mess);
            return NULL;
        } */
        strncpy(read_message[i].text, messages[i].text, MAX_LENGTH_MESSAGE - 1);
        read_message[i].text[MAX_LENGTH_MESSAGE - 1] = '\0';
        strncpy(read_message[i].username, messages[i].username, 32 - 1);
        read_message[i].username[32 - 1] = '\0';

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


//Funzione per liberare i messaggi dalla memoria
void libera_messaggi(Message * messaggi, int num_messaggi) {
    for (int i = 0; i < num_messaggi; i++) {
        free(messaggi[i].text);
        free(messaggi[i].username);
    }
    free(messaggi);
}
