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

int registra_bool = 0; // Per vedere se è loggato o no

//Funzione per aggiungere client alla lista
void add_client(listaGiocatori* lista, int client_fd, char* username){
    giocatore *new_giocatore = (giocatore *)malloc(sizeof(giocatore));
    new_giocatore->username = strdup(username);
    new_giocatore->client_fd = client_fd;
    new_giocatore->tid = pthread_self();
    // strcpy(new_giocatore->username, username);
    
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

//Funzione per controllare se username è già esistente
int username_esiste(listaGiocatori* lista, char *username){
    if (lista == NULL || lista->head == NULL){
        return 0;
    }
    giocatore *current = lista->head;
    while (current != NULL){
        if (strcmp(current->username, username) == 0){
            return 1;
        }
        current = current->next;
    }
    return 0;
}

// Funzione per registrazione del cliente
void registrazione_client(int client_fd, char *username, listaGiocatori *lista){
    // Controllo se l'username contiene solo caratteri validi
    if (!controlla_caratteri(username)){
        send_message(client_fd, MSG_ERR, "Username non valido, non deve contenere caratteri ASCII");
        return; 
    }

    // Controllo se l'username esiste già
    if (username_esiste(lista, username)){
        printf("Tentativo di registrazione con username già presente: %s\n", username);
        fflush(0);
        send_message(client_fd, MSG_ERR, "Username già in uso, per favore scegli un altro nome");
        return; 
    }

    add_client(lista, client_fd, username);
    //setto l'username come usato -> impostare booleano nella lista ad 1
    // Se l'username è valido, invio un messaggio di conferma
    send_message(client_fd, MSG_OK, "Registrazione avvenuta con successo");
    registra_bool = 1;
}

int login_utente(int client_fd,listaGiocatori *lista, char *username){
    giocatore *current = lista->head;
    if (current -> active == 1){
        send_message(client_fd, MSG_ERR, "Login già avvenuto, non è possibile registrarsi");
    }
    //controllo se l'utente è già registrato
    if(registra_bool== 1){
        send_message(client_fd, MSG_ERR, "Una registrazione è avvenuta, non è possibile registrarsi nuovamente");
        return 1;
    }
    //controllo se l'utente è in gioco
    if (!username_esiste(lista, username)){
        return 1; 
    }
    //controllo se il nome utente è gia preso da qualcuno
    //loggo l'utente
     send_message(client_fd, MSG_OK, "Login avvenuto con successo");
    current -> active = 1;
    return 0;
}

// //Funzione per stampare la lista dei client
void stampa_lista_clienti(Fifo *lista){
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

void elimina_giocatore(listaGiocatori *lista, char *username){
   // pthread_mutex_lock(&lista_mutex);
    giocatore *corrente = lista->head;
    giocatore *precedente = NULL;

    while (corrente != NULL){
        if (strcmp(corrente->username, username) == 0){
            printf("Trovato giocatore da eliminare: %s\n", username);
            fflush(0);
            if (precedente == NULL){
                lista->head = corrente->next;
            } else {
                precedente->next = corrente->next;
            }
            if (corrente == lista->tail) {
                lista->tail = precedente;
            }
            free(corrente->username);
            free(corrente);
            lista->count--;
            // pthread_mutex_unlock(&lista_mutex);
            printf("Giocatore eliminato con successo: %s\n", username);
            fflush(0);
            return;
        }
        precedente = corrente;
        corrente = corrente->next;
    }
   // pthread_mutex_unlock(&lista_mutex);
    printf("Giocatore non trovato: %s\n", username);
    fflush(0);
}
/*
void elimina_thread(Fifo *clients, pthread_t thread_id, pthread_mutex_t *clients_mutex){
    pthread_mutex_lock(clients_mutex);
    Client *current = clients->head;
    Client *prev = NULL;

    while (current != NULL) {
        if (pthread_equal(current->thread_id, thread_id) != 0) { // Usa thread_id invece di clients_mutex
            pthread_cancel(current->thread_id);
            pthread_join(current->thread_id, NULL);
            if (prev == NULL) {
                clients->head = current->next;
            } else {
                prev->next = current->next;
            }
            free(current->username);
            free(current);
            pthread_mutex_unlock(clients_mutex);
            return;
        }
        prev = current;
        current = current->next;
    }
    pthread_mutex_unlock(clients_mutex);
}
*/
/* Funzione che recupera il nome utente di un giocatore dalla lista
giocatore* RecuperaUtente(listaGiocatori* newLista, char* username){
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
    //Non ho trovato il nome utente dentro la lista
    pthread_mutex_unlock(&newLista->lock);
    return NULL;
}
*/
void elimina_thread(Fifo *clients, pthread_t thread_id, pthread_mutex_t *clients_mutex) {
    pthread_mutex_lock(clients_mutex);
    Client *current = clients->head;
    Client *prev = NULL;

    while (current != NULL) {
        if (pthread_equal(current->thread_id, thread_id)) { 
            printf("[SERVER] Eliminazione del thread per il client %s (tid: %ld)\n", 
            current->username ? current->username : "Sconosciuto", thread_id);

            // Chiude il socket del client
            if (current->fd >= 0) {
                printf("[SERVER] Chiudo il socket del client %s (fd: %d)\n", current->username, current->fd);
                close(current->fd);
                //current->fd = -1;
            }


            // Cancella il thread solo se è ancora attivo
            pthread_cancel(current->thread_id);
            pthread_join(current->thread_id, NULL);

            // Rimuove il client dalla lista
            if (prev == NULL) {
                clients->head = current->next;
            } else {
                prev->next = current->next;
            }

            // Libera la memoria
            free(current->username);
            free(current);
            pthread_mutex_unlock(clients_mutex);
            return;
        }
        prev = current;
        current = current->next;
    }

    pthread_mutex_unlock(clients_mutex);
}

