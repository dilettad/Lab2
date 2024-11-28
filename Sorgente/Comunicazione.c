#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h> 
#include <arpa/inet.h>
#include <pthread.h>
#include "../Header/macro.h"
#include "../Header/Trie.h"
#include "../Header/Lista.h"
#include "../Header/Comunicazione.h"


//Funzione per inviare un messaggio
void send_message(int client_socket, char type, char* data) {
    int retvalue;
    //invio del campo data
    SYSC(retvalue,write(client_socket,data,sizeof(data)),"nell'invio del payload")
}

//Funzione per ricevere un messaggio
message receive_message(int client_socket){
    message msg;int retvalue;
    msg.data = (char*)malloc(msg.length + 1); //Alloca in memoria lo spazio necessario per il data
    SYSC(retvalue,read(client_socket,msg.data,msg.length),"nella ricezione del payload");
    return msg;
}
/*
//GIOCATORE : danno tutti problemi
//Funzione per aggiungere client alla lista
void add_client(Fifo* lista, int client_fd, char* username){
     Client* new_client = malloc(sizeof(Client));
     // Inizializza i membri della struttura Client
        new_client->fd = client_fd; // Assegna il file descriptor
        strncpy(new_client->username, username, sizeof(new_client->username) - 1);
        new_client->username[sizeof(new_client->username) - 1] = '\0'; // Assicurati che sia null-terminated
        new_client->next = NULL;
}
  
/* Funzione per registrazione del cliente --> VENGONO FUORI DEGLI ERRORI
void registrazione_client(int client_fd, char* username){
     Client* new_client = client_fd; 
     // Controllo se l'username è già presente
     while(new_client != NULL) {
         // Se l'username è già presente, manda un messaggio di errore
         if (strcmp(new_client->username, username) == 0){
             send_message(client_fd , MSG_ERR ,"Username già esistente, scegline un altro");
             break; //Esco dalla funzione
         }
         new_client = new_client ->next;
    }
     // Controllo se l'username contiene solo caratteri validi
         if (!controlla_caratteri(client_fd)){
            send_message(client_fd, MSG_ERR, "Username non valido, non deve contenere caratteri ASCII");
            return; // Esci dalla funzione se l'username non è valido
        }
     // Se l'username è valido, invio un messaggio di conferma 
    send_message(client_fd,MSG_OK,"username valido");

     //Inserisco il client nella lista e ne incremento il numero
    add_client(&clients_head, client_fd,username );
    num_client++;
}
*/
/*
//Controllo caratteri dell'username: non deve contenere caratteri ASCII
int controlla_caratteri(const char* username){
     while(*username){
        if((unsigned char)*username < 128){ // Controlla se il carattere è ASCII
             return 0; // Se contiene caratteri ASCII, restituisci 0
        }
        username++;
     }
     return 1; // Se tutti i caratteri sono validi (non ASCII), restituisci 1
}
*/
/*Funzione per stampare la lista dei client
void stampa_lista_clienti(Fifo* Lista){
    Client * current = lista -> head; // Devo mettere Lista -> head?
    while (current != NULL) {
        printf("client_fd: %d, Username: %s\n", current -> fd, current -> username);
        current = current->next;
    }
}
*/
/*
// Funzione per il login di un utente
void login_utente(Fifo* lista, const char* username, int client_fd) {
    Client *current = lista->head;

    // Cerca il nome utente nella lista
    while (current != NULL) {
        if (strcmp(current->username, username) == 0 && current->fd != -1) {
            // Nome utente trovato e non cancellato
            send_message(client_fd, MSG_LOGIN_UTENTE);
            return; // Uscita dalla funzione dopo un login riuscito
        }
        current = current->next;
    }

    // Se il ciclo termina, l'utente non è stato trovato
    // send_message(client_fd, MSG_ERR);
}
*/

/*
// Funzione per cancellare l'utente dopo una disconnessione
void cancella_registrazione(Fifo*  Lista) {
    Client *current = lista -> head;
    Client *prev = NULL;

    while (current != NULL) { 
        if (current->fd == -1) { // Nodo da rimuovere trovato
            if (prev == NULL) {  // Il nodo da rimuovere è il primo nodo della lista
                lista -> head = current->next;
            } else { // Il nodo da rimuovere non è il primo nodo
                prev->next = current->next;
            }
            // Liberiamo la memoria del nodo corrente
            Client *temp = current; // Salviamo il puntatore al nodo corrente
            current = current->next; // Aggiorniamo current prima di free
            free(temp); // Liberiamo la memoria
            num_client--; // Decrementiamo il conteggio dei clienti
            // invio del messaggio MSG_CANCELLA_UTENTE
            printf(MSG_CANCELLA_UTENTE);
        } else {
            // Aggiorniamo prev e current solo se non abbiamo rimosso il nodo
            prev = current;
            current = current->next;
        }

    }

}
*/









