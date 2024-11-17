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

void InputStringa(cella** matrix, char*string);

void stampaMatrice(cella** matrix);

int trovaParola(cella** matrice, char* parola );

int trovaPos(int x, int y, char lettera,cella** matrice);