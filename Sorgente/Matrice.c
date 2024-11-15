#include <stdio.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "../Header/Matrice.h"

#define MATRIX_SIZE 4

//Generazione matrice
cella** generateMatrix(){
    cella** matrix = (cella**)malloc(MATRIX_SIZE*sizeof(cella*));
    for(int i = 0; i < MATRIX_SIZE; i++){
        matrix[i] = (cella*)malloc(MATRIX_SIZE*sizeof(cella));
    }
    return matrix;
}

//Funzione che prende in input una stringa per la matrice
void InputStringa(cella** matrix, char*string){
    int k = 0;
    for (int i = 0; i < 4; i++){
        for(int j = 0; j < 4; i++ ){
            matrix[i][j].value = string[k];
            k++;
            matrix[i][j].usato = false;
        }
    }
}
    


//Funzione per stampare la matrice
void stampaMatrix(){

}



//Funzione per cercare in matrice

