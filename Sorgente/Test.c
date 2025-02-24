#include <stdio.h>
#include <errno.h>

#include "../Header/Matrice.h"
#include "../Header/Lista.h"
#include "../Header/Giocatore.h"
#include "../Header/FunzioniServer.h"
#include "../Header/Trie.h"

int main(int argc, char* argv[]){

// TEST SU MATRICE.C    
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
    //svuotaMatrice(matrice);

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


//TEST SU TRIE.C
// Inizializza il Trie
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
    return 0;
}
 
// TEST GIOCATORE
    /*int risultato =  controlla_caratteri("DJ0Q..");
    printf("%d\n",risultato);
    
    
    // char username[5] = "test";    
    // Fifo* lista = create();

    //Prova registrazione_client
   
    // printf("%d\n", risultato);
    // //stampa utente
    // printf("Lista dei clienti dopo la registrazione:\n");
    //stampa_lista_clienti();



    TEST SERVER
    TEST SERVER
    time_t tempo_iniziale = time(NULL) - 18; // Simula che il gioco è iniziato 18 secondi fa
    int durata_partita = 20;    
    char* messaggio = calcola_tempo_rimanente(tempo_iniziale, durata_partita);
    printf("%s\n", messaggio);  // Usa %s per stampare una stringa
    return 0;
    

#include <stdio.h>
#include "../Header/Matrice.h"
#include "../Header/Lista.h"

int main(int argc, char* argv[]) {
    cella*** matrice = generateMatrix();
    if (matrice == NULL) {
        fprintf(stderr, "Errore nella generazione della matrice\n");
        return 1;
    }

    InputStringa(matrice, "ATLCIOQADVESISBI");
    stampaMatrice(matrice);
    
    printf("%d\n", trovaParola(matrice, "CASE"));

    int client_fd = 1;
    char username[20]; // Aumenta la dimensione se necessario
    strncpy(username, "test", sizeof(username) - 1);
    username[sizeof(username) - 1] = '\0'; // Assicurati che sia terminato

    Fifo* lista = create();
    registrazione_client(client_fd, username);
    
    // Stampa utente
    printf("Lista dei clienti dopo la registrazione:\n");
    stampa_lista_clienti();

    // Libera la memoria, se necessario
    // freeMatrix(matrice);
    
    return 0;
}
*/