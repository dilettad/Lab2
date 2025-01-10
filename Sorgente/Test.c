#include <stdio.h>
#include <errno.h>

#include "../Header/Matrice.h"
#include "../Header/Lista.h"
#include "../Header/Giocatore.h"
#include "../Header/FunzioniServer.h"
#include "../Header/Trie.h"

// Calcola tempo rimanente
char*  calcola_tempo_rimanente(time_t tempo_iniziale, int durata_partita) {
    time_t tempo_attuale = time(NULL);
    double tempo_trascorso = difftime(tempo_attuale, tempo_iniziale);
    printf("Il tempo trascorso è: %f secondi\n", tempo_trascorso);
    double tempo_rimanente = durata_partita - tempo_trascorso;
    printf("Il tempo rimanente è: %f secondi\n", tempo_rimanente);
    // Se il tempo rimanente è minore di 0 allora vuol dire che il gioco è finito
    if (tempo_rimanente < 0) {
        return "Il gioco è già terminato\n";
    } 

    // Calcolo lunghezza del messaggio e alloco memoria
    int length = 64;
    char* messaggio = (char*)malloc(length + 1);

    // Verifica se l'allocazione è riuscita
    if (messaggio == NULL) {
        return "Errore di allocaione della memoria \n";
    }
    //Scrive il messaggio formattato nella memeoria allocata
    snprintf(messaggio, length+1, "Il tempo rimanente è: %f secondi\n", tempo_rimanente);  
    //return il messaggio
    return messaggio;  
}


int main(int argc, char* argv[]){

// TEST SU MATRICE.C    

    printf("Inizio test matrice \n");
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
    printf("Fine test matrice \n");

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

// TEST SU SERVER.C
    printf("TEST SU SERVER \n");
    time_t tempo_iniziale = time(NULL);
    int durata_partita = 30; // durata della partita in secondi 
    
    // Simula un'attesa di 3 secondi
    sleep(3);
    
    char* risultato = calcola_tempo_rimanente(tempo_iniziale, durata_partita);
    printf("%s", risultato);
    
    // Libera la memoria allocata
    free(risultato);



    return 0;
}