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
    // Quindi testa e coda sono NULL, quindi per assegnare il nuovo cliente aggiorno la coda
    lista->tail = new_client;
    }
    new_client->next = lista->head;
    lista->head = new_client;
    lista->size++;
}

//Client * pop (Fifo * lista){ da errore per il return
Client * pop (Fifo * list){
    // Controllo se la lista è vuota

    if (list->size == 0){
        // Se vuota allora non devo uscire quindi return null
        return NULL;
    }
    // Controllo se la lista è di dimensione 1 perchè in questo caso avrei 1-1 = 0
    if (list->size == 1){
        list->head = list->tail;
        list->tail = NULL;
        list->size = 0;
        return list->tail;
    }
    // Ciclo fino all'elemento priam della coda
    Client* temp = list->head;
    for (int i = 0; i < list->size - 1; i++){
        temp = temp->next;
    }
    // Aggiorno la coda
    list->tail = temp;  
    return temp; 
}

int seek (Fifo * list, char* username){// 1 se trova un cliente con quel username 0 altrimenti
    Client* temp = list->head;
    while (temp != NULL){
        if (temp -> username != NULL && strcmp(temp->username, username) == 0){
            return 1;
        }
        temp = temp->next;
    }
    return 0;
}

//Funzione sulla lista dei giocatori, aggiorna punteggio
void aggiorna_punteggio(listaGiocatori* lista, char* username, int punteggio){
    giocatore* current = lista -> head;
    while(current != NULL){
        if (strcmp(current->username, username) == 0){
            current->punteggio = punteggio;
            return;
        }
    current = current -> next;        
    }
}    

/* Funzione per eliminare la lista dei giocatori 
void distruggi_lista (listaGiocatori* lista){
    giocatore* current = lista->head;
    giocatore* next = NULL;
    while (current != NULL){
        next = current->next;
        free(current);
        current = next;
        }
        lista->head = NULL;
        lista->tail = NULL;
        lista->count = 0;
}
*/

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

