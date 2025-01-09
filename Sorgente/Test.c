#include <stdio.h>
#include <errno.h>

#include "../Header/Matrice.h"
#include "../Header/Lista.h"
#include "../Header/Giocatore.h"
int main(int argc, char* argv[]){
    
    // TEST MATRICE
    int retvalue;
    cella** matrice = generateMatrix();
    // writef(retvalue,"casa\n");
    InputStringa(matrice,"ATLCIOQADVESISBI");
    stampaMatrice(matrice);
    
    printf("%d\n",trovaParola(matrice, "CASE"));
    printf("Esecuzione dei test --- \n");
    // TEST GIOCATORE
    /*int risultato =  controlla_caratteri("DJ0Q..");
    printf("%d\n",risultato);
    */
    // char username[5] = "test";    
    // Fifo* lista = create();

    //Prova registrazione_client
   
    // printf("%d\n", risultato);
    // //stampa utente
    // printf("Lista dei clienti dopo la registrazione:\n");
    //stampa_lista_clienti();



    //TEST SERVER
    time_t tempo_iniziale = time(NULL) - 18; // Simula che il gioco è iniziato 18 secondi fa
    int durata_partita = 20;    
    char* messaggio = calcola_tempo_rimanente(tempo_iniziale, durata_partita);
    printf("%s\n", messaggio);  // Usa %s per stampare una stringa
    return 0;
}

/*
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