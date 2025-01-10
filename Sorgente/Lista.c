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
#include "../Header/FunzioniServer.h"

// Creo la prima lista assegnando la testa e la coda del cliente
Fifo * create (){
    Fifo* lista = (Fifo*) malloc(sizeof(Fifo));
    lista->head = NULL;
    lista->tail = NULL;
    lista->size = 0;
    return lista;
}

// Funzione per aggiungere un nuovo cliente alla lista
void push (Fifo * lista, Client *new_client){
    // Controllo se la lista è vuota
    if (lista->size == 0){
    //Testa e coda sono NULL, assegno il nuovo cliente come  coda
    lista->tail = new_client;
    }
    new_client->next = lista->head; //Collega il nuovo cliente alla testa
    lista->head = new_client; //Aggiorno la testa della lista
    lista->size++; 
}

//Funzione rimuove e restituisce il cliente dalla coda - NON CORRETTA
/* Client * pop (Fifo * list){
    if (list->size == 0){ //Se lista vuota restituisce NULL
        return NULL;
    }
    // Controllo se la lista è di dimensione 1 
    if (list->size == 1){
        Client * temp = list->head; 
        list->head = NULL; //Assegna la testa alla coda 
        list->tail = NULL; //Imposta la coda come NULL
        list->size = 0; // Imposta la dimensione a 0
        return temp; // Restituisce il cliente
    }
    // Ciclo fino all'elemento prima della coda
    Client* temp = list->head;
    for (int i = 0; i < list->size - 1; i++){
        temp = temp->next; 
    }
    // Aggiorno la coda
    list->tail = temp;  
    return temp; 
}*/

// Funzione rimuove e restituisce il cliente dalla coda
Client * pop (Fifo * list) {
    if (list->size == 0) { // Se lista vuota restituisce NULL
        return NULL;
    }
    Client * temp = list->head; // Salva il cliente da restituire
    list->head = list->head->next; // Aggiorna la testa della lista
    if (list->head == NULL) { // Se la lista è vuota dopo la rimozione
        list->tail = NULL; // Imposta la coda come NULL
    }
    list->size--; // Decrementa la dimensione della lista
    return temp; // Restituisce il cliente
}

// Funzione cerca un cliente nella lista in base all'username
int seek (Fifo * list, char* username){// 1 se trova un cliente con quel username, 0 altrimenti
    Client* temp = list->head; // Inizializzo il puntatore alla testa della lista
    while (temp != NULL){
        if (temp -> username != NULL && strcmp(temp->username, username) == 0){
            return 1; // Return 1 se trova un cliente con lo stesso username
        }
        temp = temp->next; //Successivo
    }
    return 0; //Non trova cliente
}

//Funzione aggiorna punteggio del giocatore 
void aggiorna_punteggio(listaGiocatori* lista, char* username, int punteggio){
    giocatore* current = lista -> head;
    while(current != NULL){
        if (strcmp(current->username, username) == 0){ //Controllo se corrisponde l'username del giocatore con quello corrente
            current->punteggio = punteggio; // Aggiorna punteggio
            return;
        }
    current = current -> next; //Successivo  
    }
}    

// Funzione per eliminare la lista dei giocatori 
void distruggi_lista (listaGiocatori* lista){
    giocatore* current = lista->head; //Inizializza un puntatore alla testa della lista
    giocatore* next = NULL; //Inizilizza un puntatore per il nodo successivo
    while (current != NULL){ //
        next = current->next; 
        free(current); 
        current = next;
        }
        lista->head = NULL;
        lista->tail = NULL;
        lista->count = 0;
}


/*
 //Funzione per eliminare la lista dei giocatori 
 void distruggi_lista(listaGiocatori * lista) {
    pthread_mutex_lock(&lista->lista_mutex);
    giocatore *current = lista->head;
    while (current != NULL) {
        giocatore *temp = current;
        current = current->next;
        free(temp);
    }
    lista->head = NULL;
    pthread_mutex_unlock(&lista->lista_mutex);
    pthread_mutex_destroy(&lista->lista_mutex);
}
*/
