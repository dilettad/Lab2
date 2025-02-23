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
#include "../Header/Giocatore.h"

// Funzione per creare una lista FIFO
Fifo *create(){
    Fifo *lista = (Fifo *)malloc(sizeof(Fifo));               //Alloca la memoria per una nuova struttura
    lista->head = NULL;                                       //Inizializza testa e coda a NULL
    lista->tail = NULL;
    lista->size = 0;                                          //Inizializzo il contatore a 0
    return lista;
}

// Funzione per aggiungere un nuovo cliente alla lista Fifo
void push(Fifo *lista, Client *new_client){
    if (lista->size == 0){                                    //Controllo se lista è vuota in tal caso la testa punta al nuovo client
        lista->tail = new_client;
    }
    new_client->next = lista->head;                           //Punto il nuovo client alla testa della lista
    lista->head = new_client;                                 //Punto la testa della lista al nuovo client
    lista->size++;                                            //Incrementa la dimensione della lista
}

// Funzione per eliminare un cliente dalla lista
void deleteClient(Fifo *lista, pthread_t tid) {

    if (lista->head == NULL) {                                //Controlla se la testa dalla lista è vuota
        return;
    }

    //Inizializza il puntatore current alla testa della lista e prev a NULL
    Client *current = lista->head;
    Client *prev = NULL;

    //Scansione lista 
    while (current != NULL) {
        if (current->thread_id == tid) {                      //Controllo se tid e thread_id corrispondono
            if (prev == NULL) {                               //Se devo eliminare la testa, aggiono la testa al prossimo
                lista->head = current->next;
            } else {                                          //altrimenti, collego il precedente al successivo
                prev->next = current->next;
            }

            if (current == lista->tail) {                     //Se devo eliminare la coda, aggiorno la coda al precedente
                lista->tail = prev;
            }

            if(current->username!=NULL){                      //Libera memoria allocata per il l'username del client
                free(current->username);
            }

            free(current);                                    //Libera memoria per il client e decrementa il contatore 
            lista->size--;
            return;
        }

    //Aggiorna il prev con il corrente
        prev = current;
        if(current->next == NULL){                           // Se il successivo è NULL, allora return
            return;
        }    
        current = current->next;                            //Punto il puntatore al successivo
    }
}
