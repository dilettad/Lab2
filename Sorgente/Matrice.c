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

// Funzione per convertire la stringa in maiuscolo
void Caps_Lock(char* string){
    int len = strlen(string);                                               //Lunghezza stringa

    //Scorro la stringa
    for(int i =0;i<len;i++){
        if (string[i]>= 'a' && string[i]<= 'z'){                            //Controllo se carattere è minuscolo
            string[i]-= 32;                                                 //Converto in maiuscolo
        }
    }
    return;
}

// Funzione per generare una matrice
cella **generateMatrix(){
    cella **matrice = (cella **)malloc(MATRIX_SIZE * sizeof(cella *));      // Allocazione memoria le celle della matrice
    for (int i = 0; i < MATRIX_SIZE; i++){
        matrice[i] = (cella *)malloc(MATRIX_SIZE * sizeof(cella));          // Allocazione memoria le celle della riga
    }
    return matrice;
}

// Funzione crea una matrice dalla stringa passata
void InputStringa(cella **matrice, char *string){
    int k = 0;                                                              //Indice per scorrere
    
    // Scansione della matrice, per riga e colonna
    for (int i = 0; i < MATRIX_SIZE; i++){
        for (int j = 0; j < MATRIX_SIZE; j++){
            matrice[i][j].value = string[k];                                //Assegna il carattere della stringa alla cella
            k++;                                                            //Incremento indice
            matrice[i][j].usato = false;                                    //Imposta la cella come non usata
        }

    }
}


// Funzione stampa matrice
void stampaMatrice(cella **matrice){

    // Scansione della matrice, per riga e colonna
    for (int i = 0; i < MATRIX_SIZE; i++){
        for (int j = 0; j < MATRIX_SIZE; j++){
            if (matrice[i][j].value == 'Q' && j + 1 < MATRIX_SIZE ){        //Controllo se il valore della stringa è uguale a Q
                printf("Qu ");                                              //Stampa carattere "Qu" nella cella
            } else {
            printf("%c ", matrice[i][j].value);                             //Stampa valore nella cella
            }
        }
        printf("\n");
    }
}

//Funzione ricorsiva per cercare una parola nella matrice
int trovaParolaAux(cella** matrice, int i, int j, char* parola, int index) {
    
    if (index >= strlen(parola)) {                                          
        return 1;                                                           //Se verificato tutta la parola, allora esiste
    }

    // Controlla i limiti della matrice
    if (i < 0 || i >= MATRIX_SIZE || j < 0 || j >= MATRIX_SIZE) {
        return 0;
    }
    //Controllo se sono già passata sopra quella cella
    if (matrice[i][j].usato){
        return 0;
    }
    //Caso speciale: gestione della lettera "Q"
    if (matrice[i][j].value == 'Q' && index + 1 < strlen(parola) && parola[index] == 'Q' && parola[index + 1] == 'U') {
        printf("DEBUG: Trovata 'QU' in (%d, %d)\n", i, j);

        matrice[i][j].usato = true;
        //Cerco in tutte le 8 direzioni adiacenti
        return trovaParolaAux(matrice, i + 1, j, parola, index + 2) ||  
               trovaParolaAux(matrice, i - 1, j, parola, index + 2) ||  
               trovaParolaAux(matrice, i, j + 1, parola, index + 2) ||  
               trovaParolaAux(matrice, i, j - 1, parola, index + 2) ||  
               trovaParolaAux(matrice, i + 1, j + 1, parola, index + 2) ||  
               trovaParolaAux(matrice, i - 1, j - 1, parola, index + 2) ||  
               trovaParolaAux(matrice, i + 1, j - 1, parola, index + 2) ||  
               trovaParolaAux(matrice, i - 1, j + 1, parola, index + 2);  
    }

    //Se il valore della cella è diversa da quella della parola, non viene trovata
    if (matrice[i][j].value != parola[index]) {
        return 0;
    }
    printf("DEBUG: Lettera %c trovata in (%d, %d), index %d\n", parola[index], i, j, index);
    matrice[i][j].usato = true;
    // Controlla in tutte le 8 direzioni adiacenti
    return trovaParolaAux(matrice, i + 1, j, parola, index + 1) ||  
           trovaParolaAux(matrice, i - 1, j, parola, index + 1) || 
           trovaParolaAux(matrice, i, j + 1, parola, index + 1) ||  
           trovaParolaAux(matrice, i, j - 1, parola, index + 1) ||  
           trovaParolaAux(matrice, i + 1, j + 1, parola, index + 1) || 
           trovaParolaAux(matrice, i - 1, j - 1, parola, index + 1) ||  
           trovaParolaAux(matrice, i + 1, j - 1, parola, index + 1) ||  
           trovaParolaAux(matrice, i - 1, j + 1, parola, index + 1); 
    
    matrice[i][j].usato = false;        
}

//Funzione per cercare parola nella matrice
int trovaParola(cella** matrice, char* parola) {
    printf("DEBUG: Verifica della parola %s nella matrice\n", parola);
    
    int trovata = 0;                                                        //Variabile per indicare se la parola è stata trovata
    //Scorro matrice per riga e colonna
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            if (matrice[i][j].value == parola[0]) {                         //Se la prima lettera corrisponde, inizia la ricerca ricorsiva
                printf("DEBUG: Prima lettera trovata in (%d, %d)\n", i, j);
                if (trovaParolaAux(matrice, i, j, parola, 0)) {
                    trovata = 1;
                    break;
                }
            }
        }
    }

    if (!trovata) {                                                         //Parola non è stata trovata
        printf("DEBUG: La parola %s NON è nella matrice!\n", parola);
    }

    return trovata;
}

// Funzione per creare la matrice da un file
void Carica_MatricedaFile(FILE *file, cella **matrice){
    
    if (file == NULL){                                                      //Controllo esistenza del file
        printf("Errore di apertura del file\n");
        return;
    }
    
    //Scansione la matrice per riga e colonne
    for (size_t i = 0; i < MATRIX_SIZE; i++){
        for (size_t j = 0; j < MATRIX_SIZE; j++){            
            fscanf(file, " %c", &matrice[i][j].value);                      //Leggo il carattere dal file e lo memorizzo nella cella della matrice
            Caps_Lock(&matrice[i][j].value);                                //Converto in maiuscolo
            matrice[i][j].usato = false;                                    //Imposta il flag in non usato
            if (matrice[i][j].value == 'Q' ){                               //Controllo se il valore e Q, in quel caso ignora il carattere successivo
                fscanf(file, " %*c" );
            }
        }
    }
} 

//Funzione per controllare se la parola cercata è già stata trovata
int esiste_paroleTrovate(paroleTrovate *lista, char *parola) {
   
    pthread_mutex_lock(&parole_trovate_mutex);
    paroleTrovate *current = lista;                                         //Inizializzo puntatore corrente alla lista

    //Scansione lista
    while (current != NULL) {
        if (strcmp(current->parola, parola) == 0) {                         //Se la parola corrisponde a quella cercata
            pthread_mutex_unlock(&parole_trovate_mutex);
            return 1;                                                       // Parola trovata
        }
        current = current->next;
    }
    pthread_mutex_unlock(&parole_trovate_mutex);
    return 0;                                                               // Parola non trovata
}

//Funzione per aggiungere alla lista la parola trovata
paroleTrovate* aggiungi_parolaTrovata(paroleTrovate *lista, char *parola) {
    pthread_mutex_lock(&parole_trovate_mutex);
    paroleTrovate *nuova_parola = malloc(sizeof(paroleTrovate));            //Alloca memoria per nuovo elemento della lista
    if (nuova_parola == NULL) {                                             //Controllo se la parola è NULL, restituendo la lista originale
        perror("Errore di allocazione memoria");
        pthread_mutex_unlock(&parole_trovate_mutex);
        return lista;
    }

    nuova_parola->parola = strdup(parola);                                  //Copio la parola nella struttura
    nuova_parola->next = lista;                                             
    pthread_mutex_unlock(&parole_trovate_mutex);
    return nuova_parola;
}

// Invio matrice al client
void invio_matrice(int client_fd, cella **matrix){
    char *data = matrice_to_string(matrix, 4);                              //Conversione matrice in stringa
    printf("Invio matrice al client %d\n", client_fd); 
    send_message(client_fd, MSG_MATRICE, data);                             //Invia la matrice al client
}

//Funzione per convertire una matrice in una stringa
char *matrice_to_string(cella **matrix, int size){
    int length = size * size + 1;                                           //Calcoliamo la lunghezza totale della stringa (con '\0' finale)
    char *result = (char *)malloc(length * sizeof(char));                   //Alloca memoria per la stringa

    int index = 0;  
    //Scansione                                                       
    for (int i = 0; i < size; i++){
        for (int j = 0; j < size; j++){
            result[index++] = matrix[i][j].value;                           //Copia il valore della cella nella stringa
        }
    }
    result[index] = '\0';                                                   //Terminatore di stringa
    return result;
}
