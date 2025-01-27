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


// GIOCATORE : deve essere messo su client o su lista?

/*Funzione per aggiungere client alla lista
void add_client(Fifo* lista, int client_fd, char* username){
     Client* new_client = (Client*) malloc(sizeof(Client));
     new_client -> fd = client_fd;
     strcpy(new_client->username, username);
     new_client->next = NULL;
}
*/

// Controllo caratteri dell'username: non deve contenere caratteri ASCII
int controlla_caratteri(const char *username)
{
    while (*username)
    {
        if ((unsigned char)*username >= 128)
        {             // Controlla se il carattere è ASCII
            return 0; // Se contiene caratteri ASCII, restituisci 0
        }
        username++;
    }
    return 1; // Se tutti i caratteri sono validi (non ASCII), restituisci 1
}
// TESTATA: FUNZIONA

// Funzione per registrazione del cliente
void registrazione_client(int client_fd, char *username, listaGiocatori *lista)
{
    // // Controllo se l'username contiene solo caratteri validi
    // if (!controlla_caratteri(username))
    // {
    //     send_message(client_fd, MSG_ERR, "Username non valido, non deve contenere caratteri ASCII");
    //     return; // Esci dalla funzione se l'username non è valido
    // }
    // if (lista == NULL || lista->head == NULL)
    // {
    //     send_message(client_fd, MSG_ERR, "Lista vuota");
    //     return;
    // }
    // Client *current = lista->head;

    // // Controllo se l'username è già presente
    // while (current != NULL)
    // {
    //     // Se l'username è già presente, manda un messaggio di errore
    //     if (strcmp(current->username, username) == 0)
    //     {
    //         send_message(client_fd, MSG_ERR, "Username già esistente, sceglierne un altro");
    //         return;
    //     }

    //     current = current->next;
    // }

    // Se l'username è valido, invio un messaggio di conferma
    // send_message(client_fd, MSG_OK, "Username valido");

    // Inserisco il client nella lista e ne incremento il numero
    // add_client(&clients_head, client_fd,username );
    //  Inserisco il client nella lista

    // push(lista->head, username);
    //  Controllo se la lista è vuota
    // creo il giocatore
    giocatore *new_giocatore = (giocatore *)malloc(sizeof(giocatore));
    new_giocatore->username = strdup(username);
    new_giocatore->client_fd = client_fd;
    new_giocatore->punteggio = 0;
    new_giocatore->next = NULL;

    if (lista->count == 0)
    {
        // Testa e coda sono NULL, assegno il nuovo cliente come  coda
        lista->tail = new_giocatore;
    }
    new_giocatore->next = lista->head; // Collega il nuovo cliente alla testa
    lista->head = new_giocatore;       // Aggiorno la testa della lista

    lista->count++;
    return;
}

// //Funzione per stampare la lista dei client
void stampa_lista_clienti(Fifo *lista)
{
    if (lista->head == NULL)
    {
        printf("Lista dei clienti vuota\n");
        fflush(0);
        return;
    }
    Client *current = lista->head;
    while (current != NULL)
    {
        printf("client_fd: %d, Username: %s\n", current->fd, current->username);
        fflush(0);
        current = current->next;
    }
}


// TESTATA: FUNZIONA

/* Funzione per eliminare il giocatore dalla lista
void elimina_giocatore(Fifo* lista, int client_fd){
    if (lista == NULL || lista->head == NULL) {
        send_message(client_fd, MSG_ERR, "Lista vuota");
        return;
    }

    Client* current = lista->head;
    Client* previous = NULL;

    // Cerca il client da eliminare
    while (current != NULL) {
        if (current->fd == client_fd) {
            // Se il client da eliminare è il primo della lista
            if (previous == NULL) {
                lista->head = current->next;
            } else {
                previous->next = current->next;
            }

            // Se il client da eliminare è l'ultimo della lista
            if (current->next == NULL) {
                lista->tail = previous;
            }

            free(current);
            printf("Client con username %d eliminato.\n", client_fd);
            return;
        }

        previous = current;
        current = current->next;
    }

    printf("Client con username %d non trovato.\n", client_fd);
}*/

void elimina_giocatore(listaGiocatori *lista, char *username, pthread_mutex_t lista_mutex)
{
    pthread_mutex_lock(&lista_mutex);
    giocatore *corrente = lista->head;
    giocatore *precedente = NULL;

    while (corrente != NULL)
    {
        if (strcmp(corrente->username, username) == 0)
        {
            if (precedente == NULL)
            {
                lista->head = corrente->next;
            }
            else
            {
                precedente->next = corrente->next;
            }
            lista->count--;
            pthread_mutex_unlock(&lista_mutex);
            return;
        }
        precedente = corrente;
        corrente = corrente->next;
    }
    pthread_mutex_unlock(&lista_mutex);
    // se il giocatore non è stato trovato, non fa nulla
}


/*
void elimina_thread(Fifo * lista, int fd, pthread_mutex_t *clients_mutex){
    pthread_mutex_lock(&clients_mutex);
    Client *current = lista->head;
    Client *precedente = NULL;

    while(current != NULL){
        if(pthread_equal(current->fd, clients_mutex) != 0){
            if(precedente == NULL){
                lista->head = current->next;

                if (lista -> head == NULL){
                lista->tail = NULL;
                }

            }
            else{
                precedente->next = current->next;
            }
            lista->size--;
            free(current -> username);
            free(current);
            pthread_mutex_unlock(&clients_mutex);
            return;
        }
        precedente = current;
        current = current->next;
    }
    pthread_mutex_unlock(&clients_mutex);
}
*/



void elimina_thread(Fifo *clients, pthread_t thread_id, pthread_mutex_t *clients_mutex) {
    pthread_mutex_lock(clients_mutex);
    Client *current = clients->head;
    Client *prev = NULL;

    while (current != NULL) {
        if (pthread_equal(current->thread_id, thread_id) != 0) { // Usa thread_id invece di clients_mutex
            if (prev == NULL) {
                clients->head = current->next;
            } else {
                prev->next = current->next;
            }
            free(current);
            break;
        }
        prev = current;
        current = current->next;
    }
    pthread_mutex_unlock(clients_mutex);
}

/*Funzione che recupera il nome utente di un giocatore -> NON MI PRENDE LISTA
listaGiocatori RecuperaUtente(listaGiocatori* newLista, char* username) {
    pthread_mutex_lock(&newLista->lock);
    // listaGiocatori head = newLista->lista;
    giocatore* lista = newLista->head;
    //Fino a che la lista non è vuota, controllo se il nome utente coincide con quello nella lista
    while (lista != NULL) {
        if (strcmp(lista->username, username) == 0) {
            pthread_mutex_unlock(&newLista->lock);
            return lista; //?
        }
        lista = lista->next;
    }
   // newLista->lista = head;
    //Non ho trovato il nome utente dentro la lista
    pthread_mutex_unlock(&newLista->lock);
    return NULL;
}
*/
