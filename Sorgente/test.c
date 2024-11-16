#include "../Header/Matrice.h"

int main(int argc, char* argv[]){

    cella** matrice = generateMatrix();
    
    InputStringa(matrice,"ATLCIOQADVESISBI");
   
    stampaMatrice(matrice);
    printf("ciao\n");
    printf( "%d",trovaParola(matrice, "CASE"));
    return 0;
}