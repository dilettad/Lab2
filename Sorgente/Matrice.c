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
    cella** matrix = (cella**)malloc(MATRIX_SIZE*sizeof(cella*));
    for(int i = 0; i < MATRIX_SIZE; i++){
        matrix[i] = (cella*)malloc(MATRIX_SIZE*sizeof(cella));
    }
    return matrix;
}

//Funzione che prende in input una stringa per la matrice
void InputStringa(cella** matrix, char*string)
{
    int k = 0;
    for (int i = 0; i < MATRIX_SIZE; i++){
        for(int j = 0; j < MATRIX_SIZE; j++ ){
            matrix[i][j].value = string[k];
            k++;
            
            matrix[i][j].usato = false;
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
int trovaParolaAux(cella*** matrice, int i, int j, char* parola, int index) 
{
    int cose[] = {0,1,0,-1,1,0,-1,0,1,1,1,-1,-1,1,-1,-1};
    if (index >= strlen(parola)) 
    {
        return 1;
    }
    
    //IL PROBLEMA STA NEL CICLO FOR CHE NON TERMINA PERCHè LA FUNZIONE ENTRA IN RICORSIONE MA NON ABBASTANZA PER DUMPARE IL CORE
    for (size_t c = 0; c < 16; c=+2)
    {
        
        //printf("qui\n");
        int x = i + cose[c];
        int y = j + cose[c+1];
        //printf("i:%d j:%d c:%d,qui\n",i,j,c);
        //IL PROBLEMA STA NEL CICLO FOR CHE NON TERMINA
        if (x < MATRIX_SIZE && y < MATRIX_SIZE && x > 0 && y > 0)
        {
            //printf("index: %d, x: %d, y: %d qui\n",index,x,y);
            //printf("%c\n",(*matrice[x][y]).value);
            if ((*matrice[x][y]).value == parola[index])
            {
                //printf("qui\n");
                //printf("%c\n",(*matrice[x][y]).value);
                (*matrice[x][y]).usato = true;
                if (trovaParolaAux(matrice, x, y, parola, index+1))
                    return 1;
                (*matrice[x][y]).usato = false;
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

