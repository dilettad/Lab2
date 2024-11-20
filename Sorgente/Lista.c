#include <stdio.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "../Header/macro.h"
#include "../Header/comunicazione.h"
#include "../Header/Trie.h"
#include "../Header/Lista.h"


// Puntatore alla testa della lista di clienti
Client *clients_head = NULL;

Client clients [MAX_CLIENTS]; //Array di clienti con un massimo di 32 clienti
int num_client = 0; //Numero di clienti connessi    

//Funzione per aggiungere client alla lista
void add_client(Client** head, char*username){
    if (num_client >= MAX_CLIENTS){
        printf("Errore: numero di clienti massimo raggiunto\n");
        return;
    }
    Client* new_client = &clients[num_client];
    strcpy(new_client->username, username);
    new_client->next = *head;
    *head = new_client;
}    
        
//Funzione per registrazione del cliente
void registrazione_client(int client_fd,char* username){
    Client* current = clients_head;
    // Controllo se l'username è già presente
    while(current != NULL) {
        if (strcmp(current->username, username) == 0) {
            send_message(client_fd,MSG_ERR,"Username già esistente, scegline un altro");
            break;
        }
        current = current ->next;
    }
    // Controllo se l'username contiene solo caratteri validi
    if (!controlla_caratteri(username)) {
        send_message(client_fd, MSG_ERR, "Username non valido, non deve contenere caratteri ASCII");
        return; // Esci dalla funzione se l'username non è valido
    }
    // Se l'username è valido, invio un messaggio di conferma 
    send_message(client_fd,MSG_OK,"username valido");

    //Inserisco il client nella lista e ne incremento il numero
    add_client(&clients_head, username);
    num_client++;
}  

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



//Funzione per stampare la lista dei client
void stampa_lista_clienti(){
    Client* current = clients_head;
    while (current != NULL) {
        printf("client_fd: %d\n", current -> username);
        current = current->next;
        }
}




