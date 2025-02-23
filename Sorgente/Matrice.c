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
pthread_mutex_t parole_trovate_mutex = PTHREAD_MUTEX_INITIALIZER;
// DA SISTEMARE STAMPA MATRICE 

void Caps_Lock(char* string){
    //recupero la lunghezza della stringa
    int len = strlen(string);
    //ciclo sulla stringa
    for(int i =0;i<len;i++){
        //controllo se il carattere è in lower case
        if (string[i]>= 'a' && string[i]<= 'z'){
            //lo porto in uppercase
            string[i]-= 32;
        }
    }
    return;
}


// Funzione per generare una matrice
cella **generateMatrix(){
    // Allocazione memoria le celle della matrice
    cella **matrice = (cella **)malloc(MATRIX_SIZE * sizeof(cella *));
    for (int i = 0; i < MATRIX_SIZE; i++)
    {
        // Allocazione memoria le celle della riga
        matrice[i] = (cella *)malloc(MATRIX_SIZE * sizeof(cella));
    }
    return matrice;
}

// Funzione crea una matrice dalla stringa passata
void InputStringa(cella **matrice, char *string){
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


// Funzione stampa matrice
void stampaMatrice(cella **matrice){
    // Scansione della matrice, per riga e colonna
    for (int i = 0; i < MATRIX_SIZE; i++)
    {
        // printf("entro ciclio i:%d j:%d\n",i,j);
        for (int j = 0; j < MATRIX_SIZE; j++){
            if (matrice[i][j].value == 'Q' && j + 1 < MATRIX_SIZE ){
                printf("Qu "); // Stampa valore della cella
            }
            else
            printf("%c ", matrice[i][j].value); // Stampa valore della cella
            // printf("i:%d, j:%d\n",i,j);
        }
        
        printf("\n");
    }
    // return;
}

int trovaParolaAux(cella** matrice, int i, int j, char* parola, int index) {
    if (index >= strlen(parola)) {
        return 1;  // Se abbiamo verificato tutta la parola, allora esiste
    }

    // Controlla i limiti della matrice
    if (i < 0 || i >= MATRIX_SIZE || j < 0 || j >= MATRIX_SIZE) {
        return 0;
    }

    //Caso speciale: gestione della lettera "Q"
    if (matrice[i][j].value == 'Q' && index + 1 < strlen(parola) && parola[index] == 'Q' && parola[index + 1] == 'U') {
        printf("DEBUG: Trovata 'QU' in (%d, %d)\n", i, j);
        return trovaParolaAux(matrice, i + 1, j, parola, index + 2) ||  
               trovaParolaAux(matrice, i - 1, j, parola, index + 2) ||  
               trovaParolaAux(matrice, i, j + 1, parola, index + 2) ||  
               trovaParolaAux(matrice, i, j - 1, parola, index + 2) ||  
               trovaParolaAux(matrice, i + 1, j + 1, parola, index + 2) ||  
               trovaParolaAux(matrice, i - 1, j - 1, parola, index + 2) ||  
               trovaParolaAux(matrice, i + 1, j - 1, parola, index + 2) ||  
               trovaParolaAux(matrice, i - 1, j + 1, parola, index + 2);  
    }


    if (matrice[i][j].value != parola[index]) {
        return 0;
    }

    printf("DEBUG: Lettera %c trovata in (%d, %d), index %d\n", parola[index], i, j, index);

    // Controlla in tutte le 8 direzioni possibili
    return trovaParolaAux(matrice, i + 1, j, parola, index + 1) ||  
           trovaParolaAux(matrice, i - 1, j, parola, index + 1) || 
           trovaParolaAux(matrice, i, j + 1, parola, index + 1) ||  
           trovaParolaAux(matrice, i, j - 1, parola, index + 1) ||  
           trovaParolaAux(matrice, i + 1, j + 1, parola, index + 1) || 
           trovaParolaAux(matrice, i - 1, j - 1, parola, index + 1) ||  
           trovaParolaAux(matrice, i + 1, j - 1, parola, index + 1) ||  
           trovaParolaAux(matrice, i - 1, j + 1, parola, index + 1);  
}

int trovaParola(cella** matrice, char* parola) {
    printf("DEBUG: Verifica della parola %s nella matrice\n", parola);
    
    int trovata = 0;  // Variabile per indicare se la parola è stata trovata

    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            if (matrice[i][j].value == parola[0]) {  // Se la prima lettera corrisponde
                printf("DEBUG: Prima lettera trovata in (%d, %d)\n", i, j);
                if (trovaParolaAux(matrice, i, j, parola, 0)) {
                    trovata = 1;
                    break;
                }
            }
        }
    }

    if (!trovata) {
        printf("DEBUG: La parola %s NON è nella matrice!\n", parola);
    }

    return trovata;
}



// Funzione per creare la matrice da un file
void Carica_MatricedaFile(FILE *file, cella **matrice){
    // Controllo del file, esistenza
    if (file == NULL){
        printf("Errore di apertura del file\n");
        return;
    }
    // Carico dal file la matrice
    for (size_t i = 0; i < MATRIX_SIZE; i++){
        for (size_t j = 0; j < MATRIX_SIZE; j++){
            // fscanf, scansiona e legge il file e lo memorizza nella matrice come valore
            
            fscanf(file, " %c", &matrice[i][j].value);

            Caps_Lock(&matrice[i][j].value); // Trasforma in maiuscolo

            // Cella disponibile per inserimento, in quanto ancora non usata
            matrice[i][j].usato = false;
            if (matrice[i][j].value == 'Q' ){
                fscanf(file, " %*c" );
                
            }
        }
    }
    // Carico la matrice dal file
    // InputStringa(matrice, file);
} 

// Funzione per svuotare la matrice
void svuotaMatrice(cella **matrice){
    for (int i = 0; i < MATRIX_SIZE; i++){
        free(matrice[i]);
    }
    free(matrice);
}

int esiste_paroleTrovate(paroleTrovate *lista, char *parola) {
    pthread_mutex_lock(&parole_trovate_mutex);
    paroleTrovate *current = lista;
    while (current != NULL) {
        if (strcmp(current->parola, parola) == 0) {
            pthread_mutex_unlock(&parole_trovate_mutex);
            return 1; // Parola trovata
        }
        current = current->next;
    }
    pthread_mutex_unlock(&parole_trovate_mutex);
    return 0; // Parola non trovata
}

paroleTrovate* aggiungi_parolaTrovata(paroleTrovate *lista, char *parola) {
    pthread_mutex_lock(&parole_trovate_mutex);
    paroleTrovate *nuova_parola = malloc(sizeof(paroleTrovate));
    if (nuova_parola == NULL) {
        perror("Errore di allocazione memoria");
        pthread_mutex_unlock(&parole_trovate_mutex);
        return lista;
    }
    nuova_parola->parola = strdup(parola);
    nuova_parola->next = lista;
    pthread_mutex_unlock(&parole_trovate_mutex);
    return nuova_parola;
}

// Invio matrice al client attraverso un socket
void invio_matrice(int client_fd, cella **matrix){
    char *data = matrice_to_string(matrix, 4); // Conversione matrice in stringa
    // printf("%s\n", data);
    printf("Invio matrice al client %d\n", client_fd); // Stampa messaggio
    send_message(client_fd, MSG_MATRICE, data);        // Invia la matrice al client
}

char *matrice_to_string(cella **matrix, int size){
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
