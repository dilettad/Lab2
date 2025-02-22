#ifndef MATRICE_H
#define MATRICE_H




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

typedef struct paroleTrovate{
    char* parola;
    struct paroleTrovate* next;
}paroleTrovate;


cella** generateMatrix();

void InputStringa(cella** matrice, char* string);

void stampaMatrice(cella** matrice);

int trovaParola(cella** matrice, char* parola );

int trovaParolaAux(cella** matrice, int i, int j, char* parola, int index);

void Carica_MatricedaFile(FILE* file, cella** matrice);

void svuotaMatrice(cella** matrice);

//int esiste_paroleTrovate(paroleTrovate* head, const char* parola);
int esiste_paroleTrovate(paroleTrovate *lista, char *parola);
//paroleTrovate* aggiungi_parolaTrovata(paroleTrovate* head, const char* parola);
paroleTrovate* aggiungi_parolaTrovata(paroleTrovate *lista, char *parola);
void libera_paroleTrovate(paroleTrovate *lista);

char* matrice_to_string(cella** matrix, int size);

void invio_matrice(int client_fd, cella** matrix);

#endif // MATRICE_H
