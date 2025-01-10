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
// in lista struttura client
#include "../Header/Giocatore.h"

//GIOCATORE : deve essere messo su client o su lista?

//Funzione per aggiungere client alla lista
void add_client(Fifo* lista, int client_fd, char* username){
     Client* new_client = (Client*) malloc(sizeof(Client));
     new_client -> fd = client_fd;
     strcpy(new_client->username, username);
     new_client->next = NULL;   
} 
// TESTATA: FUNZIONA

// Controllo caratteri dell'username: non deve contenere caratteri ASCII
int controlla_caratteri(const char* username){
     while(*username){
        if((unsigned char)*username >= 128){ // Controlla se il carattere è ASCII
             return 0; // Se contiene caratteri ASCII, restituisci 0
        }
        username++;
     }
     return 1; // Se tutti i caratteri sono validi (non ASCII), restituisci 1
}
// TESTATA: FUNZIONA

//Funzione per registrazione del cliente
void registrazione_client(int client_fd, char* username, Fifo* lista){
     // Controllo se l'username contiene solo caratteri validi
        if (!controlla_caratteri(username)) {
            send_message(client_fd, MSG_ERR, "Username non valido, non deve contenere caratteri ASCII");
            return; // Esci dalla funzione se l'username non è valido
        }
        if (lista == NULL || lista->head == NULL){
            send_message(client_fd, MSG_ERR, "Lista vuota");
            return;
        }
    Client* current = lista -> head;
    // Controllo se l'username è già presente
    while(current != NULL) {
         // Se l'username è già presente, manda un messaggio di errore
         if (strcmp(current->username, username) == 0) {
             send_message(client_fd,MSG_ERR,"Username già esistente, scegline un altro");
             return;
         }
         current = current -> next;
    }
        
     // Se l'username è valido, invio un messaggio di conferma 
    send_message(client_fd,MSG_OK,"Username valido");

    //Inserisco il client nella lista e ne incremento il numero
    //add_client(&clients_head, client_fd,username );
    // Inserisco il client nella lista
    push(lista -> head, username);
    // listaGiocatori.count ++;
    return ;
}

// //Funzione per stampare la lista dei client
void stampa_lista_clienti(Fifo* lista){
    if (lista->head == NULL){
        printf("Lista dei clienti vuota\n");
        return;
    }
    Client* current = lista -> head;
    while (current != NULL) {
        printf("client_fd: %d, Username: %s\n", current -> fd, current -> username);
        current = current->next;
    }
}
//TESTATA: FUNZIONA

// Serve un libera lista? Non posso fare un free(lista)
