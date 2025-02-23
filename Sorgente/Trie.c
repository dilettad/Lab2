#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "../Header/macro.h"
#include "../Header/Trie.h"
#include "../Header/Trie.h"
#include <stdio.h>
#include <stdlib.h>

#include "../Header/Trie.h"
#include <stdio.h>
#include <stdlib.h>

// Crea un nodo del Trie
Trie *create_node(){
    Trie *trie = (Trie *)malloc(sizeof(Trie));      //Alloca memoria per un nuovo nodo
    if (trie){
        trie->is_word = -1;                         //Flag che indica che la parola non è terminata
        //Scansione 
        for (int i = 0; i < NUM_CHAR; i++){
            trie->figli[i] = NULL;                  //Inizializza puntatore figli[i] a NULL
        }
    }
    return trie;
}

// Inserisci una parola nel Trie
int insert_Trie( Trie* trie,  char* word){
    //Caso base: se Trie NULL, crea un nodo
    if (trie == NULL){
        trie = create_node();
    }
    //Caso base: se parola è vuota imposta il flag a 0 e termina
    if (*word == '\0'){
        trie->is_word = 0;
        return 0;
    }

    int target_child = *word - 'A';                          //Cerco il figlio successivo, se non esiste lo creo
    if (trie->figli[target_child] == NULL){             
        trie->figli[target_child] = create_node();
    }
   
    return insert_Trie(trie->figli[target_child],word+1);   //Chiamata ricorsiva: inserire il resto della parola nel sottoalbero figlio
}

//Cerco una parola nel Trie
int search_Trie( char* word, Trie* trie){
    //Caso base: il trie è vuoto
    if (trie == NULL)return 0;
    printf("word: %s\n", word);

    //Caso base: sono arrivato alla fine della parola
    if (*word == '\0'){
        return trie->is_word;
    }
    
    int index = *word -'A';                                 //Calcolo indice del figlio 

    if (trie->figli[index] == NULL){                        //Non esiste il figlio allora non è presente la parola
        return -1;
    }

    return search_Trie(word+1,trie->figli[index]);          //Chiamata ricorsiva per cercare il resto della parola nel sottoalbero figlio
}


// Stampa il Trie
void Print_Trie(Trie *trie, char *buffer, int depth){
    //Caso base: il Trie è null
    if (trie == NULL){
        printf("l'albero è vuoto\n");
        return;
    }
    //Nodo corrente corrisponde alla fine di una parola, stampandola
    if (trie->is_word == 0){
        buffer[depth] = '\0';
        printf("%s\n", buffer);
    }
    //Scansione
    for (int i = 0; i < NUM_CHAR; i++){
        if (trie->figli[i] != NULL){                            //Se figlio esiste, aggiunge carattere al buffer
            buffer[depth] = i + 'A';
            Print_Trie(trie->figli[i], buffer, depth + 1);      //Chiamata ricorsiva per stampare il resto della parola
        }
    }
}