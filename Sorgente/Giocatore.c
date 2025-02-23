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

int registra_bool = 0; // Per vedere se è registrato o no

//Funzione per aggiungere client alla lista
void add_client(listaGiocatori* lista, int client_fd, char* username){
    // Allocazione di memoria per un nuovo giocatore
    giocatore *new_giocatore = (giocatore *)malloc(sizeof(giocatore));
  
    new_giocatore->username = strdup(username);   //Duplicazione dell'username
    new_giocatore->tid = pthread_self();          //Assegnazione tid al thread corrente
    new_giocatore->next = NULL;                   // Inizializzato puntatore al giocatore successivo a NULL

    if (lista->count == 0){                       // Testa e coda sono NULL, assegno il nuovo cliente come  coda
        lista->tail = new_giocatore;
    }
    new_giocatore->next = lista->head;            // Collega il nuovo cliente alla testa
    lista->head = new_giocatore;                  // Aggiorno la testa della lista
    lista->count++;                               // aggiorno contatore della lista
    return;
}

// Controllo caratteri dell'username: non deve contenere caratteri ASCII
int controlla_caratteri(const char *username){
    while (*username){
        if ((unsigned char)*username >= 128){   // Controlla se il carattere è ASCII
            return 0;                          
        }
        username++;
    }
    return 1;                                   // Se tutti i caratteri sono validi (non ASCII), restituisci 1
}

//Funzione per controllare se username è già esistente
int username_esiste(listaGiocatori* lista, char *username){
    //Controllo se la lista è vuota
    if (lista == NULL || lista->head == NULL){ 
        return 0;
    }
    // Inizializzo il puntatore current alla testa della lista
    giocatore *current = lista->head;
    while (current != NULL){ 
        if (strcmp(current->username, username) == 0){ // Se corrisponde vuol dire che esiste
            return 1; 
        }
        current = current->next;    
    }

    return 0;
}

// Funzione per registrazione del cliente
void registrazione_client(int client_fd, char *username, listaGiocatori *lista){
    // Controllo se l'username contiene solo caratteri validi e mando un messaggio in caso di errore
    if (!controlla_caratteri(username)){
        send_message(client_fd, MSG_ERR, "Username non valido, non deve contenere caratteri ASCII");
        return; 
    }

    // Controllo se l'username esiste già e mando un messaggio in caso di errore
    if (username_esiste(lista, username)){
        printf("Tentativo di registrazione con username già presente: %s\n", username);
        fflush(0);
        send_message(client_fd, MSG_ERR, "Username già in uso, per favore scegli un altro nome");
        return; 
    }

    add_client(lista, client_fd, username); //Aggiungo alla lista se passa i controlli, mando un messaggio di OK
    send_message(client_fd, MSG_OK, "Registrazione avvenuta con successo");
    registra_bool = 1;
}

//Funzione per loggare l'utente
int login_utente(int client_fd, listaGiocatori *lista, char *username){

    //Inizializzo il puntatore current alla testa della lista
    giocatore *current = lista->head;

    // Scansione la lista per trovare il giocatore
    while (current != NULL) {
        if (strcmp(current->username, username) == 0) { // Controllo corrisponde l'username
            if (current->active == 1) {                 // Controllo se è attivo e in tal caso mando un messaggio di Errore
                send_message(client_fd, MSG_ERR, "Sei già loggato");
                return 1;
            }
        // Se non risulta attivo lo loggo e cambio il flag active in 1, mandando un messaggio di OK
        current->active = 1;
        send_message(client_fd, MSG_OK, "Login avvenuto con successo");
            return 0;
        }
        current = current->next;                       
    }
    send_message(client_fd, MSG_ERR, "Errore: username non trovato.");
    return 1;
}

//Funzione per eliminare il giocatore dalla lista dei giocatori
void elimina_giocatore(listaGiocatori *lista, char *username){

    //Inizializzo corrente con la testa della lista e precedente a NULL
    giocatore *corrente = lista->head;
    giocatore *precedente = NULL;

    //Scansione lista per trovare il giocatore 
    while (corrente != NULL){
        if (strcmp(corrente->username, username) == 0){                 //Controllo se corrisponde l'username
            printf("Trovato giocatore da eliminare: %s\n", username);   
            fflush(0);

            if (precedente == NULL){                                   
                lista->head = corrente->next;                           //Punto la testa della lista al successivo
            } else {
                precedente->next = corrente->next;                      //Punto il precedente al prossimo giocatore
            }
            if (corrente == lista->tail) {                              //Se giocatore è alla coda
                lista->tail = precedente;                               //Punto la coda della lista al precedente
            }
            free(corrente->username);                                   //Libero la memoria allocata e decremento il contantore delle lista
            free(corrente);
            lista->count--;

            printf("Giocatore eliminato con successo: %s\n", username);
            fflush(0);
            return;
        }
        precedente = corrente;                                          //Aggiorna il puntatore precedente
        corrente = corrente->next;                                      //Passa al prossimo giocatore
    }
}

//Funzione per eliminare un thread dalla lista dei clients dopo un periodo di inattività
void elimina_thread(Fifo *clients, pthread_t thread_id, pthread_mutex_t *clients_mutex) {

    //Inizializzo current alla testa dei clients e prev a NULL
    Client *current = clients->head;
    Client *prev = NULL;
    
    // Scansione dei clients
    while (current != NULL) {
        if (pthread_equal(current->thread_id, thread_id)) {                                 //Controllo corrispondenza tra ID e thread corrispondente
           
            printf("[SERVER] Eliminazione del thread per il client %s (tid: %ld)\n", current->username ? current->username : "Sconosciuto", thread_id);
            send_message(current->fd, MSG_FINE, "Sei stato disconnesso per inattività.");   //Invio messaggio di disconessione
     
            pthread_cancel(current->thread_id);                                             //Cancellazione del thread
            pthread_join(current->thread_id, NULL);                                         // Attende terminazione del thread 

            printf("[SERVER] Chiudo il socket del client %s (fd: %d)\n", current->username, current->fd);
            close(current->fd);                                                             //Chiusura del socket del client
            
            // Rimuove il client dalla lista
            if (prev == NULL) {
                clients->head = current->next;                                              //Punta testa dei client al prossimo
            } else {
                prev->next = current->next;                                                 //Punta prev al prossimo 
            }

            free(current->username);                                                        // Libera la memoria
            free(current);
            return;
        }
        prev = current;                                                                     //Aggiorna puntatore prev e passa al prossimo
        current = current->next;
    }
}

