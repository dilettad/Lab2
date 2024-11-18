#include <stdio.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdbool.h> 
#include "../Header/macro.h"
#include "../Header/Matrice.h"

#define MATRIX_SIZE 4

//Generazione matrice
cella** generateMatrix()
{
    cella** matrice = (cella**)malloc(MATRIX_SIZE*sizeof(cella*));
    for(int i = 0; i < MATRIX_SIZE; i++){
        matrice[i] = (cella*)malloc(MATRIX_SIZE*sizeof(cella));
    }
    return matrice;
}

//Funzione che prende in input una stringa per la matrice
void InputStringa(cella** matrice, char*string)
{
    int k = 0;
    for (int i = 0; i < MATRIX_SIZE; i++){
        for(int j = 0; j < MATRIX_SIZE; j++ ){
            matrice[i][j].value = string[k];
            k++;
            
            matrice[i][j].usato = false;
        }
    }
} 

//Funzione per stampare la matrice
void stampaMatrice(cella** matrice)
{
    for(int i = 0; i < MATRIX_SIZE; i++){
        for(int j = 0; j < MATRIX_SIZE; j++){
            printf("%c ", matrice[i][j].value);
        }
        printf("\n");
    }
    return ;
}

int trovaParolaAux(cella** matrice, int i, int j, char* parola, int index) 
{
    int cose[] = {0,1,0,-1,1,0,-1,0,1,1,1,-1,-1,1,-1,-1};
    if (index >= strlen(parola)) 
    {
        return 1;
    }
   
    for (size_t c = 0; c < 16; c+=2) 
    {
        int x = i + cose[c];
        int y = j + cose[c+1];
        
        if (x < MATRIX_SIZE && y < MATRIX_SIZE && x >= 0 && y >= 0) 
        {
            if (matrice[x][y].value == parola[index]) 
            {
                matrice[x][y].usato = true;
                if (trovaParolaAux(matrice, x, y, parola, index+1))
                    return 1;
                matrice[x][y].usato = false;
                return 0;
            }
        }
    }
   return 0;
}

//Funzione per cercare sulla matrice
int trovaParola(cella** matrice, char* parola )
{
    for (size_t i = 0; i < MATRIX_SIZE; i++)
    {
        
        for (size_t j = 0; j < MATRIX_SIZE; j++)
        {
            
           if (matrice[i][j].value == parola[0])   
            {
                
                matrice[i][j].usato = true;
                if (trovaParolaAux(&matrice, i, j, parola,1))
                    return 1;
                matrice[i][j].usato = false;
            }
        }
    }
    return 0;
}

