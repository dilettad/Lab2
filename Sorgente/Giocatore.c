#include <stdio.h>
#include <stdbool.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "../Header/macro.h"
#include "../Header/Comunicazione.h"
#include "../Header/Trie.h"
#include "../Header/Lista.h"
#include "../Header/Giocatore.h"

//GIOCATORE : deve essere messo su client o su lista?
//Funzione per aggiungere client alla lista
void add_client(Fifo* lista, int client_fd, char* username){
     Client* new_client = (Client*) malloc(sizeof(Client));
     new_client -> fd = client_fd;
     strcpy(new_client->username, username);
     new_client->next = NULL;   
}
  
//Funzione per registrazione del cliente
void registrazione_client(int client_fd, char* username){
     Client* current = clients_head;
     // Controllo se l'username è già presente
     while(current != NULL) {
         // Se l'username è già presente, manda un messaggio di errore
         if (strcmp(current->username, username) == 0) {
             send_message(client_fd,MSG_ERR,"Username già esistente, scegline un altro");
             break; //Esco dalla funzione
         }
         current = current ->next;
    }
     // Controllo se l'username contiene solo caratteri validi
         if (!controlla_caratteri(client_fd)) {
            send_message(client_fd, MSG_ERR, "Username non valido, non deve contenere caratteri ASCII");
            return; // Esci dalla funzione se l'username non è valido
        }
     // Se l'username è valido, invio un messaggio di conferma 
    send_message(client_fd,MSG_OK,"username valido");

     //Inserisco il client nella lista e ne incremento il numero
    add_client(&clients_head, client_fd,username );
    num_client++;
}

// //Controllo caratteri dell'username: non deve contenere caratteri ASCII
int controlla_caratteri(const char* username){
     while(*username){
        if((unsigned char)*username < 128){ // Controlla se il carattere è ASCII
             return 0; // Se contiene caratteri ASCII, restituisci 0
        }
        username++;
     }
     return 1; // Se tutti i caratteri sono validi (non ASCII), restituisci 1
}

// //Funzione per stampare la lista dei client
void stampa_lista_clienti(){
    Client* current = clients_head;
    while (current != NULL) {
        printf("client_fd: %d, Usernale: %s\n", current -> fd, current -> username);
        current = current->next;
    }
}


