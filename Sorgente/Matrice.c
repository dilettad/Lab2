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
//Generazione matrice
cella** generateMatrix(){
    // Allocazione memoria le celle della matrice
    cella** matrice = (cella**)malloc(MATRIX_SIZE*sizeof(cella*));
    for(int i = 0; i < MATRIX_SIZE; i++){
        // Allocazione memoria le celle della riga
        matrice[i] = (cella*)malloc(MATRIX_SIZE*sizeof(cella));
    }
    return matrice;
}

//Funzione che prende in input una stringa per la matrice
void InputStringa(cella** matrice, char* string){
    // Scansione della stringa
    int k = 0;
    // Scansione della matrice, per riga e colonna
    for (int i = 0; i < MATRIX_SIZE; i++){
        for(int j = 0; j < MATRIX_SIZE; j++ ){
            matrice[i][j].value = string[k];
            k++;
            //printf("%c ",matrice[i][j].value);
            // Cella già visitata
            matrice[i][j].usato = false;
        }
        //printf("\n");
    }    
} 

//Funzione per stampare la matrice
void stampaMatrice(cella** matrice){
    int j = 0;
    //Scansione della matrice, per riga e colonna
    for(int i = 0; i < MATRIX_SIZE; i++){
        //printf("entro ciclio i:%d j:%d\n",i,j);
        for(j = 0; j < MATRIX_SIZE; j++){
            //Stampa valore della cella
            
            printf("%c ", matrice[i][j].value);
            //printf("i:%d, j:%d\n",i,j);
        }
        //j = 0;
        printf("\n");
    }
    // return;
}


//
int trovaParolaAux(cella** matrice, int i, int j, char* parola, int index){
// Variazioni coordinate in base alle 8 direzionali: verticali, orizzontali e diagonali
    int cose[] = {0,1,0,-1,1,0,-1,0,1,1,1,-1,-1,1,-1,-1};
    //printf("%c\n",parola[index]);
    if (index >= strlen(parola)){
        return 1;
    }
    
    for (size_t c = 0; c < 16; c+=2){
        int x = i + cose[c];
        int y = j + cose[c+1];
      
        //printf("%c\n",matrice[x][y].value );  
        if (x < MATRIX_SIZE && y < MATRIX_SIZE && x >= 0 && y >= 0){
            if (matrice[x][y].value == parola[index] && matrice[x][y].usato == false){
                matrice[x][y].usato = true;
                if (trovaParolaAux(matrice, x, y, parola, index+1)){
                    return 1;
                matrice[x][y].usato = false;
                }
            }
        }
    }
   return 0;
}

//Funzione per cercare sulla matrice
int trovaParola(cella** matrice, char* parola ){
    for (size_t i = 0; i < MATRIX_SIZE; i++){
        for (size_t j = 0; j < MATRIX_SIZE; j++){  
           if (matrice[i][j].value == parola[0]){
                matrice[i][j].usato = true;
                if (trovaParolaAux(matrice, i, j, parola,1))
                    return 1;
                matrice[i][j].usato = false;
            }
        }
    }
    return 0;
}

//Funzione per creare la matrice da un file 
void Carica_MatricedaFile(FILE* file, cella** matrice){
    //Controllo del file, esistenza o errori 
    if (file == NULL){
        printf("Errore di apertura del file\n");
        return;
    }
    // Carico dal file la matrice
    for (size_t i = 0; i < MATRIX_SIZE; i++){
        for (size_t j = 0; j < MATRIX_SIZE; j++){
            // fscanf, scansiona e legge il file e lo memorizza nella matrice come valore
            fscanf(file, "%c", &matrice[i][j].value);
            // Cella disponibile per inserimento, in quanto ancora non usata
            matrice[i][j].usato = false;
        }
    }
    //Chiusura del file
    fclose(file);
    //Carico la matrice dal file
    //InputStringa(matrice, file);
}

//Funzione per svuotare la matrice
void svuotaMatrice(cella** matrice){
    for (int i = 0; i < MATRIX_SIZE; i++){
        for(int j = 0; j < MATRIX_SIZE; j++){
            // Metto nella cella un valore nullo
            // matrice[i][j] = '\0';
            // libero la matrice
            free(matrice[i]);
        }
    }
}        

//Controlla se parola è stata già trovata dall'utente
int esiste_paroleTrovate(paroleTrovate* head, const char* parola){
    if(head == NULL){
        return 0; //lista vuota
    }
    paroleTrovate* current = head;
    while(current != NULL){
        if(strcmp(current->parola, parola) == 0){
            return 1; //parola trovata, quindi già proposta precedentemente
        }
        current = current->next;
    }
    return 0; //parola non trovata, quindi valida
}

// Invio della matrice e del tempo rimanente in base alla fase del gioco in cui è il giocatore
void invio_matrice(int client_fd, char matrix [MATRIX_SIZE][MATRIX_SIZE]){
    int length = MATRIX_SIZE * MATRIX_SIZE;
    char data[length];
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            data [i * MATRIX_SIZE + j] = matrix[i][j];
        }
    }    
    printf("Invio matrice al client %d\n", client_fd);
    send_message(client_fd, MSG_MATRICE, data);
}

