/*
gcc server.c Lista.c Client.c Comunicazione.c -lrt -pthread
*/


#include <stdio.h>
#include "../Header/Matrice.h"
#include "../Header/Lista.h"

int main(int argc, char* argv[]){

    cella*** matrice = generateMatrix();
    
    InputStringa(matrice,"ATLCIOQADVESISBI");
   
    stampaMatrice(matrice);
    
    //printf( "%d",trovaParola(matrice, "CASE"));

    //int client_fd = 1;
    // char username[5] = "test";
    //Fifo* lista = create();
    //registrazione_client(client_fd,username);
    //stampa utente
   // printf("Lista dei clienti dopo la registrazione:\n");
    //stampa_lista_clienti();
    return 0;
  
}
