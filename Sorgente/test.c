#include <stdio.h>
#include "../Header/Matrice.h"
#include "../Header/Lista.h"

int main(int argc, char* argv[]){

    cella** matrice = generateMatrix();
    
    InputStringa(matrice,"ATLCIOQADVESISBI");
   
    stampaMatrice(matrice);
    printf("ciao\n");
    printf( "%d",trovaParola(matrice, "CASE"));

    int client_fd = 1;
    char username[5] = "test";

    registrazione_client(client_fd,username);
    //stampa utente
    printf("username: %s\n",username);
    return 0;
  
}
