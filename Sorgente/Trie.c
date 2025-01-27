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
Trie *create_node()
{
    Trie *trie = (Trie *)malloc(sizeof(Trie));
    if (trie)
    {
        trie->is_word = -1;
        for (int i = 0; i < NUM_CHAR; i++)
        {
            trie->figli[i] = NULL;
        }
    }
    return trie;
}

// Inserisci una parola nel Trie
int insert_Trie( Trie* trie,  char* word){
    //caso base
    if (trie == NULL){
        trie = create_node();
    }
    //caso base
    if (*word == '\0'){
        trie->is_word = 0;
        return 0;
        
    }
    //cerco il prossimo figlio
    int target_child = *word - 'A';
    if (trie->figli[target_child] == NULL){
        trie->figli[target_child] = create_node();
    }
    //chiamata ricorsiva
    return insert_Trie(trie->figli[target_child],word+1);
}


int search_Trie( char* word, Trie* trie){
    //caso base, il trie è vuoto
    if (trie == NULL)return 0;
    printf("word: %s\n", word);
    //sono arrivato in fondo alla parola
    if (*word == '\0'){
        return trie->is_word;
    }
    //calcolo la posizione del carattere da cercare
    int index = *word -'A';
    //printf("index: %d\n", index);
    //se non esiste il figlio -> non è presente la parola
    if (trie->figli[index] == NULL){
        return -1;
    }
    //chiamata ricorsiva
    return search_Trie(word+1,trie->figli[index]);
}


// Stampa il Trie
void Print_Trie(Trie *trie, char *buffer, int depth)
{
    if (trie == NULL)
    {
        printf("l'albero è vuoto\n");
        return;
    }
    if (trie->is_word == 0)
    {
        buffer[depth] = '\0';
        printf("%s\n", buffer);
    }
    for (int i = 0; i < NUM_CHAR; i++)
    {
        if (trie->figli[i] != NULL)
        {
            buffer[depth] = i + 'A';
            Print_Trie(trie->figli[i], buffer, depth + 1);
        }
    }
}