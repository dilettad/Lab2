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
            // Cella già visitata
            matrice[i][j].usato = false;
        }
    }    
} 

//Funzione per stampare la matrice
void stampaMatrice(cella*** matrice){
    //Scansione della matrice, per riga e colonna
    for(int i = 0; i < MATRIX_SIZE; i++){
        for(int j = 0; j < MATRIX_SIZE; j++){
            //Stampa valore della cella
            printf("%d ", matrice[i][j]->value);
        }
        printf("\n");
    }
    // return;
}
<<<<<<< HEAD
int trovaParolaAux(cella*** matrice, int i, int j, char* parola, int index) 
{
    //descrizione formali funzione
    //matrice-> la matrice di riferimento
    //i,j -> cella dell'ultima lettera convalidata
    //parola -> la parola da cercare
    //index -> riferimento alla prossima lettera da cercare nella matrice

    //int cose[] = {0,1,0,-1,1,0,-1,0,1,1,1,-1,-1,1,-1,-1};
    int x = j,y = i;
    //caso base
    if (index > strlen(parola)) 
    {
        return 1;
    }
    //clausola ricorsiva
    //provo la posizione centrale a sx
    x = j-1;
    if(trovaPos(x,i,parola[index],*matrice) == 1){
        //cerco la prossima parola
        index++;
        matrice[i][x]->usato = true;
        if(!trovaParolaAux(matrice,i,x,parola,index)){
            matrice[i][x]->usato = true;
            index++;
=======


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
      
        printf("%c\n",matrice[x][y].value );  
        if (x < MATRIX_SIZE && y < MATRIX_SIZE && x >= 0 && y >= 0){
            if (matrice[x][y].value == parola[index] && matrice[x][y].usato == false){
                matrice[x][y].usato = true;
                if (trovaParolaAux(matrice, x, y, parola, index+1)){
                    return 1;
                matrice[x][y].usato = false;
                }
            }
>>>>>>> master
        }
    }
    
    //provo la posizione centrale a dx
    x = j+1;
    if(trovaPos(x,i,parola[index],*matrice) == 1){
        //cerco la prossima parola
        index++;
        matrice[i][x]->usato = true;
        if(!trovaParolaAux(matrice,i,x,parola,index)){
            matrice[i][x]->usato = false;
            index--;
        }
    }
    //provo la posizione sopra sx
    y = i-1;x = j-1;
    if(trovaPos(x,y,parola[index],*matrice) == 1){
        //cerco la prossima parola
        index++;
        matrice[y][x]->usato = true;
        if(!trovaParolaAux(matrice,y,x,parola,index)){
            matrice[y][x]->usato = false;
            index--;
        }
    }
    //provo la posizione sopra cx
    y = i-1;
    if(trovaPos(j,y,parola[index],*matrice) == 1){
        //cerco la prossima parola
        index++;
        matrice[y][j]->usato = true;
        if(!trovaParolaAux(matrice,y,j,parola,index)){
            matrice[y][j]->usato = false;
            index--;
        }
    }
    //provo la posizione sopra dx
    y = i+1;x = j+1;
    if(trovaPos(x,y,parola[index],*matrice) == 1){
        //cerco la prossima parola
        index++;
        matrice[y][x]->usato = true;
        if(!trovaParolaAux(matrice,y,x,parola,index)){
            matrice[y][x]->usato = false;
            index--;
        }
    }
    //provo la posizione sotto sx
    y = i+1;x = j-1;
    if(trovaPos(x,y,parola[index],*matrice) == 1){
        //cerco la prossima parola
        index++;
        matrice[y][x]->usato = true;
        if(!trovaParolaAux(matrice,y,x,parola,index)){
            matrice[y][x]->usato = false;
            index--;
        }
    }
    //provo la posizione sotto cx
    y = i+1;
    if(trovaPos(j,y,parola[index],*matrice) == 1){
        //cerco la prossima parola
        index++;
        matrice[y][j]->usato = true;
        if(!trovaParolaAux(matrice,y,x,parola,index)){
            matrice[y][j]->usato = false;
            index--;
        }
    }
    //provo la posizione sotto dx
    y = i+1;x = j+1;
    if(trovaPos(x,y,parola[index],*matrice) == 1){
        //cerco la prossima parola
        index++;
        matrice[y][x]->usato = true;
        if(!trovaParolaAux(matrice,y,x,parola,index)){
            matrice[y][x]->usato = false;
            index--;
        }
    }

    
    // //IL PROBLEMA STA NEL CICLO FOR CHE NON TERMINA PERCHè LA FUNZIONE ENTRA IN RICORSIONE MA NON ABBASTANZA PER DUMPARE IL CORE
    // for (size_t c = 0; c < 16; c=+2)
    // {
        
    //     //printf("qui\n");
    //     int x = i + cose[c];
    //     int y = j + cose[c+1];
    //     //printf("i:%d j:%d c:%d,qui\n",i,j,c);
    //     //IL PROBLEMA STA NEL CICLO FOR CHE NON TERMINA
    //     if (x < MATRIX_SIZE && y < MATRIX_SIZE && x > 0 && y > 0)
    //     {
    //         //printf("index: %d, x: %d, y: %d qui\n",index,x,y);
    //         //printf("%c\n",(*matrice[x][y]).value);
    //         if ((*matrice[x][y]).value == parola[index])
    //         {
    //             //printf("qui\n");
    //             //printf("%c\n",(*matrice[x][y]).value);
    //             (*matrice[x][y]).usato = true;
    //             if (trovaParolaAux(matrice, x, y, parola, index+1))
    //                 return 1;
    //             (*matrice[x][y]).usato = false;
    //             return 0;
    //         }
    //     }
    // }
   return 0;
}


//convalida le posizioni di una matrice
int trovaPos(int x, int y, char lettera,cella** matrice/*parolaCella*/){
    printf("lettera: %c, cella:%d %d\n",lettera,y,x);
    if (x <4 && x >= 0 && y<4 && y>=0){
        cella* parolaCella = &(matrice[y][x]);
        printf("lettera:%c\n",parolaCella->value);
        if(parolaCella->value == lettera && parolaCella->usato == false){
            printf("Entrato\n");
            return 1;
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

