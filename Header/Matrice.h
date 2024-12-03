#include <stdio.h>
#include <stdbool.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "../Header/macro.h"


typedef struct cella{
    bool usato; 
    char value;
}cella;

cella** generateMatrix();

void InputStringa(cella** matrice, char* string);

void stampaMatrice(cella*** matrice);

int trovaParola(cella** matrice, char* parola );

<<<<<<< HEAD
int trovaPos(int x, int y, char lettera,cella** matrice);
=======
int trovaParolaAux(cella** matrice, int i, int j, char* parola, int index);

void Carica_MatricedaFile(FILE* file, cella** matrice);

void svuotaMatrice(cella** matrice);
>>>>>>> master
