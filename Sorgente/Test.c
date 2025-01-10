#include <stdio.h>
#include <errno.h>

#include "../Header/Matrice.h"
#include "../Header/Lista.h"
#include "../Header/Giocatore.h"
#include "../Header/FunzioniServer.h"
#include "../Header/Trie.h"

int main(int argc, char* argv[]){

// TEST SU MATRICE.C    
    printf("TEST MATRICE INIZIO \n");
     // Genera la matrice
    cella** matrice = generateMatrix();

    // Inizializza la matrice con una stringa
    char string[] = "ABCDEFGHIJKLMNOP";
    InputStringa(matrice, string);

    // Stampa la matrice
    printf("Matrice generata:\n");
    stampaMatrice(matrice);

    //Simula invio della matrice a un client
    int client_fd = 1;
    invio_matrice(client_fd, matrice);

    // Cerca una parola nella matrice
    char parola[] = "ABCD";
    if (trovaParola(matrice, parola)) {
        printf("Parola '%s' trovata nella matrice\n", parola);
    } else {
        printf("Parola '%s' non trovata nella matrice\n", parola);
    }

    // Carica una matrice da un file
    FILE* file = fopen("dizionario.txt", "r");
    Carica_MatricedaFile(file, matrice);
    printf("Matrice caricata da file:\n");
    stampaMatrice(matrice);

    // Svuota la matrice
    svuotaMatrice(matrice);

   // Inizializza la lista di parole trovate
    paroleTrovate* head = NULL;

    // Aggiungi alcune parole alla lista
    head = aggiungi_parolaTrovata(head, "HELLO");
    head = aggiungi_parolaTrovata(head, "WORLD");
    head = aggiungi_parolaTrovata(head, "TEST");

    // Verifica se alcune parole esistono nella lista
    printf("Esiste 'HELLO': %d\n", esiste_paroleTrovate(head, "HELLO")); // Dovrebbe restituire 1
    printf("Esiste 'WORLD': %d\n", esiste_paroleTrovate(head, "WORLD")); // Dovrebbe restituire 1
    printf("Esiste 'TEST': %d\n", esiste_paroleTrovate(head, "TEST")); // Dovrebbe restituire 1
    printf("Esiste 'NOTFOUND': %d\n", esiste_paroleTrovate(head, "NOTFOUND")); // Dovrebbe restituire 0

    // Stampa il contenuto della lista direttamente nel ciclo principale
    printf("Parole trovate:\n");
    paroleTrovate* current = head;
    while (current != NULL) {
        printf("%s\n", current->parola);
        current = current->next;
    }

    // Libera la memoria allocata
    current = head;
    while (current != NULL) {
        paroleTrovate* temp = current;
        current = current->next;
        free(temp->parola);
        free(temp);
    }
 printf("TEST MATRICE FINE ---- \n");

//TEST SU TRIE.C
// Inizializza il Trie
 printf("TEST TRIE INIZIO \n");
    Trie* root = create_node();

    // Inserisci alcune parole nel Trie
    insert_Trie(root, "HELLO");
    insert_Trie(root, "WORLD");
    insert_Trie(root, "HI");
    insert_Trie(root, "TRIE");
    insert_Trie(root, "TEST");

    // Cerca alcune parole nel Trie
    printf("Cerca 'HELLO': %d\n", search_Trie("HELLO", root)); // Dovrebbe restituire 0
    printf("Cerca 'WORLD': %d\n", search_Trie("WORLD", root)); // Dovrebbe restituire 0
    printf("Cerca 'HI': %d\n", search_Trie("HI", root)); // Dovrebbe restituire 0
    printf("Cerca 'TRIE': %d\n", search_Trie("TRIE", root)); // Dovrebbe restituire 0
    printf("Cerca 'TEST': %d\n", search_Trie("TEST", root)); // Dovrebbe restituire 0
    printf("Cerca 'NOTFOUND': %d\n", search_Trie("NOTFOUND", root)); // Dovrebbe restituire -1

    // Stampa il contenuto del Trie
    char buffer[256];
    printf("Contenuto del Trie:\n");
    Print_Trie(root, buffer, 0);
 printf("TEST TRIE FINE ----\n");
 
// TEST SU LISTA.C 
 printf("TEST LISTA INIZIO\n");
     Fifo* lista = create();
     Client* client1 = (Client*)malloc(sizeof(Client));
     client1->username = "user1";
     client1->next = NULL;
     push(lista, client1);

     Client* client2 = (Client*)malloc(sizeof(Client));
     client2->username = "user2";
     client2->next = NULL;
     push(lista, client2);

     printf("Seek 'user1': %d\n", seek(lista, "user1")); // Dovrebbe restituire 1
     printf("Seek 'user3': %d\n", seek(lista, "user3")); // Dovrebbe restituire 0

     Client* popped_client = pop(lista);
     printf("Popped client: %s\n", popped_client->username); // Dovrebbe restituire "user2"

     // Test delle funzioni della lista dei giocatori
    listaGiocatori giocatori;
        giocatori.head = NULL;
       pthread_mutex_init(&giocatori.lista_mutex, NULL);

     giocatore* giocatore1 = (giocatore*)malloc(sizeof(giocatore));
     giocatore1->username = "player1";
     giocatore1->punteggio = 0;
     giocatore1->next = NULL;
     giocatori.head = giocatore1;

     aggiorna_punteggio(&giocatori, "player1", 100);
     printf("Punteggio di 'player1': %d\n", giocatore1->punteggio); // Dovrebbe restituire 100

     distruggi_lista(&giocatori);

     // Pulizia
     free(client1);
     free(client2);
     free(lista);
printf("TEST LISTA FINE ----\n");


//TEST SU GIOCATORE 
printf("TEST INIZIO GIOCATORE -----\n");


printf("TEST FINE GIOCATORE -----\n");

    return 0;
}