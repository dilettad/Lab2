/*
gcc server.c Lista.c Client.c Comunicazione.c -lrt -pthread
*/

#include <stdio.h>
#include "../Header/Matrice.h"
#include "../Header/Lista.h"

int main(int argc, char* argv[]){
    printf("Esecuzione dei test --- \n");

    // cella** matrice = generateMatrix();

    // InputStringa(matrice,"ATLCIOQADVESISBI");
    
    
    // stampaMatrice(matrice);
    
    // printf( "%d",trovaParola(matrice, "CASE"));

    // int client_fd = 1;
    // char username[5] = "test";
    // Fifo* lista = create();
    // //registrazione_client(client_fd,username);
    // //stampa utente
    // printf("Lista dei clienti dopo la registrazione:\n");
    //stampa_lista_clienti();
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