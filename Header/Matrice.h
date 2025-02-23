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

//Funzione per generare una matrice
cella** generateMatrix();

//Funzione per creare una matrice da una stringa
void InputStringa(cella** matrice, char* string);

//Funzione per stampare la matrice
void stampaMatrice(cella** matrice);

//Funzione per trovare la parola all'interno della matrice
int trovaParola(cella** matrice, char* parola );

//Funzione ricorsive per trovare la parola all'interno della matrice
int trovaParolaAux(cella** matrice, int i, int j, char* parola, int index);

//Funzione per creare la matrice da un file
void Carica_MatricedaFile(FILE* file, cella** matrice);

//Funzione per convertire la stringa in maiuscolo
void Caps_Lock(char* string);

//Funzione per controllare se la parola cercata è già stata trovata
int esiste_paroleTrovate(paroleTrovate *lista, char *parola);

//Funzione per aggiungere alla lista la parola trovata
paroleTrovate* aggiungi_parolaTrovata(paroleTrovate *lista, char *parola);

//Funzione per convertire una matrice in una stringa
char* matrice_to_string(cella** matrix, int size);

//Funzione per inviare la matrice al client
void invio_matrice(int client_fd, cella** matrix);

#endif // MATRICE_H
