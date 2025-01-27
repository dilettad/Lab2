#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdbool.h>
#include "../Header/macro.h"
#include "../Header/Matrice.h"
#include "../Header/Comunicazione.h"

#define MATRIX_SIZE 4
// DA AGGIUNGERE IL CONTROLLO SU QU

// Funzione per generare una matrice
cella **generateMatrix()
{
    // Allocazione memoria le celle della matrice
    cella **matrice = (cella **)malloc(MATRIX_SIZE * sizeof(cella *));
    for (int i = 0; i < MATRIX_SIZE; i++)
    {
        // Allocazione memoria le celle della riga
        matrice[i] = (cella *)malloc(MATRIX_SIZE * sizeof(cella));
    }
    return matrice;
}
// TESTATE: FUNZIONA

// Funzione crea una matrice dalla stringa passata
void InputStringa(cella **matrice, char *string)
{
    // Scansione della stringa
    int k = 0; // Indice
    // Scansione della matrice, per riga e colonna
    for (int i = 0; i < MATRIX_SIZE; i++)
    {
        for (int j = 0; j < MATRIX_SIZE; j++)
        {
            matrice[i][j].value = string[k]; // Assegna il carattere alla cella
            k++;                             // Incremento indice
            // printf("%c ",matrice[i][j].value);
            //  Cella già visitata
            matrice[i][j].usato = false;
        }
        // printf("\n");
    }
}
// TESTATE: FUNZIONA

// Funzione stampa matrice
void stampaMatrice(cella **matrice)
{
    // int j = 0;
    // Scansione della matrice, per riga e colonna
    for (int i = 0; i < MATRIX_SIZE; i++)
    {
        // printf("entro ciclio i:%d j:%d\n",i,j);
        for (int j = 0; j < MATRIX_SIZE; j++)
        {
            printf("%c ", matrice[i][j].value); // Stampa valore della cella
            // printf("i:%d, j:%d\n",i,j);
        }
        // j = 0;
        printf("\n");
    }
    // return;
}
// TESTATE: FUNZIONA

// Funzione ricorsiva per cercare una parola
int trovaParolaAux(cella **matrice, int i, int j, char *parola, int index)
{
    // Variazioni coordinate in base alle 8 direzionali: verticali, orizzontali e diagonali
    int cose[] = {0, 1, 0, -1, 1, 0, -1, 0, 1, 1, 1, -1, -1, 1, -1, -1};
    // printf("%c\n",parola[index]);
    if (index >= strlen(parola))
    {
        return 1; // Parola trovata
    }

    for (size_t c = 0; c < 16; c += 2)
    {
        int x = i + cose[c];     // coordinata x aggiornata
        int y = j + cose[c + 1]; // coordinata y aggiornata

        // printf("%c\n",matrice[x][y].value );
        if (x < MATRIX_SIZE && y < MATRIX_SIZE && x >= 0 && y >= 0)
        {
            if (matrice[x][y].value == parola[index] && matrice[x][y].usato == false)
            {
                matrice[x][y].usato = true;
                if (trovaParolaAux(matrice, x, y, parola, index + 1))
                {
                    return 1;
                }
                matrice[x][y].usato = false;
            }
        }
    }
    return 0; // Parola non trovata
}
// TESTATE: FUNZIONA

// Funzione per cercare sulla matrice
int trovaParola(cella **matrice, char *parola)
{
    printf("%s\n", parola);
    for (size_t i = 0; i < MATRIX_SIZE; i++)
    {
        for (size_t j = 0; j < MATRIX_SIZE; j++)
        {
            if (matrice[i][j].value == parola[0])
            { // Controlla se il primo carattere corrisponde
                matrice[i][j].usato = true;
                // printf("forse\n");
                if (trovaParolaAux(matrice, i, j, parola, 1)) // Ricorsiva
                    return 1;                                 // Parola trovata
            }
            matrice[i][j].usato = false; // Se non trovata, ripristino la cella
        }
    }
    return 0; // Parola non trovata
}
// TESTATE: FUNZIONA

// Funzione per creare la matrice da un file
void Carica_MatricedaFile(FILE *file, cella **matrice)
{
    // Controllo del file, esistenza
    if (file == NULL)
    {
        printf("Errore di apertura del file\n");
        return;
    }
    // Carico dal file la matrice
    for (size_t i = 0; i < MATRIX_SIZE; i++)
    {
        for (size_t j = 0; j < MATRIX_SIZE; j++)
        {
            // fscanf, scansiona e legge il file e lo memorizza nella matrice come valore
            fscanf(file, "%c", &matrice[i][j].value);
            Caps_Lock(&matrice[i][j].value); // Trasforma in maiuscolo
            // Cella disponibile per inserimento, in quanto ancora non usata
            matrice[i][j].usato = false;
        }
    }
    // Carico la matrice dal file
    // InputStringa(matrice, file);
}
// TESTATE: FUNZIONA

// Funzione per svuotare la matrice
void svuotaMatrice(cella **matrice)
{
    for (int i = 0; i < MATRIX_SIZE; i++)
    {
        // for(int j = 0; j < MATRIX_SIZE; j++){
        //  Metto nella cella un valore nullo
        //  matrice[i][j] = '\0';
        //  libero la matrice
        free(matrice[i]);
        //}
    }
    free(matrice);
}
// TESTATE: FUNZIONA

// Controlla se parola è stata già trovata dall'utente
int esiste_paroleTrovate(paroleTrovate *head, const char *parola)
{
    if (head == NULL)
    {
        return 0; // lista vuota
    }
    paroleTrovate *current = head; // Inizializza puntatore corrente alla testa della lista
    while (current != NULL)
    {
        if (strcmp(current->parola, parola) == 0)
        {
            return 1; // parola trovata, quindi già proposta precedentemente
        }
        current = current->next;
    }
    return 0; // parola non trovata, quindi valida
}
// TESTATE: FUNZIONA

// AGGIUNGE UNA PAROLA TROVATA ALLA LISTA DI PAROLE TROVATE DA QUEL GIOCATORE DURANTE QUELLA PARTITA
paroleTrovate *aggiungi_parolaTrovata(paroleTrovate *head, const char *parola)
{
    paroleTrovate *new_node = (paroleTrovate *)malloc(sizeof(paroleTrovate)); // Alloca memoria per un nuovo nodo
    if (!new_node)
    {
        perror("Errore di allocazione della memoria\n"); // Messaggio di errore
        return head;                                     // Lista originale
    }
    new_node->parola = strdup(parola); // Duplica la stringa della parola
    if (!new_node->parola)
    {
        perror("Errore di allocazione della memoria per la parola\n"); // Messaggio di errore
        return head;                                                   // Lista originale
    }
    new_node->next = head; // Collega il nuovo nodo alla testa della lista
    return new_node;       // Restituisce il nuovo nodo
}
// TESTATE: FUNZIONA

// Invio della matrice e del tempo rimanente in base alla fase del gioco in cui è il giocatore
// Invio matrice al client attraverso un socket
void invio_matrice(int client_fd, cella **matrix)
{
    char *data = matrice_to_string(matrix, 4); // Conversione matrice in stringa
    // printf("%s\n", data);
    printf("Invio matrice al client %d\n", client_fd); // Stampa messaggio
    send_message(client_fd, MSG_MATRICE, data);        // Invia la matrice al client
}

char *matrice_to_string(cella **matrix, int size)
{
    // Calcoliamo la lunghezza totale della stringa (con '\0' finale)
    int length = size * size + 1;
    char *result = (char *)malloc(length * sizeof(char)); // Alloca memoria per la stringa

    // Copiamo i valori della matrice nella stringa
    int index = 0;
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            result[index++] = matrix[i][j].value; // Copia il valore della cella nella stringa
        }
    }
    result[index] = '\0'; // Aggiungiamo il terminatore di stringa

    return result;
}
