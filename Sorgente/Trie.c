#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "../Header/macro.h"
#include "../Header/Trie.h"

// Crea un nodo del Trie
Trie* create_node(){
    //Alloco il nodo
    Trie* trie = (Trie*)malloc(sizeof(Trie)); 
    //Inizializzo i valori della radice
    trie->is_word = -1;
    //Inizializzo i figli
    for (int i =0;i< NUM_CHAR;i++){
        trie->figli[i] = NULL; // Imposto tutti i puntatori ai figli a NULL
    }
    return trie;
}

//Cerca una parola nel Trie
int search_Trie(char* word, Trie* trie){
    //Caso base, il trie è vuoto restituisce 0
    if (trie == NULL)return 0; 
    //Sono arrivato in fondo alla parola
    if (*word == '\0'){
        return trie->is_word; // Restituisco il valore di is_word
    }

    //Calcolo la posizione del carattere da cercare (l'indice)
    int index = *word -'A'; 
    //Se non esiste il figlio -> non è presente la parola
    if (trie->figli[index] == NULL){
        return -1; 
    }
    //Chiamata ricorsiva per il carattere successivo
    return search_Trie(word+1,trie->figli[index]);
}

// Inserisce una parola nel Trie
int insert_Trie(Trie* root, char* word){
    //Caso base: nodo radice è NULL
    if (root == NULL){
        root = create_node(); //Crea un nuovo nodo
    }
    //Caso base: Arrivo alla fine della parola
    if (*word == '\0'){
        root->is_word = 0; // Parola completa
        return 0;
        
    }

    //Cerco il prossimo figlio
    int target_child = *word - 'A'; // Calcolo l'indice del carattere
    if (root->figli[target_child] == NULL){
        root->figli[target_child] = create_node();
    }
    //Chiamata ricorsiva: carattere successivo
    return insert_Trie(root->figli[target_child],word+1);
}

//Stampa tutte le parole memorizzate nel Trie
void Print_Trie(Trie* trie, char* buffer,int depth){
    //Caso base: Trie è vuoto
    if (trie == NULL){
        printf("l'albero è vuoto\n");
        return; 
    }
    //Caso base: Nodo corrente è una parola completa
    if (trie->is_word == 0){
        buffer[depth] = '\0'; // Termino la stringa nel buffer
        printf("%s\n",buffer);
    }
    //Faccio una ricerca per trovare il prossimo carattere su cui ricorrere
    for(int i = 0;i<NUM_CHAR;i++){
        if (trie->figli[i] != NULL){
            buffer[depth] = i + 'A'; // Aggiungo il carattere corrente al buffer
            Print_Trie(trie->figli[i],buffer,depth+1); //Chiamata ricorsiva 
        }
    }
    return;
}