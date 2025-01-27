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
int insert_Trie(Trie *trie, const char *word)
{
    if (trie == NULL)
    {
        fprintf(stderr, "Errore: il nodo radice è nullo\n");
        return -1;
    }

    Trie *current = trie;
    while (*word)
    {
        int target_child = *word - 'A';
        if (current->figli[target_child] == NULL)
        {
            current->figli[target_child] = create_node();
        }
        current = current->figli[target_child];
        word++;
    }
    current->is_word = 0;
    return 0;
}

// Cerca una parola nel Trie
int search_Trie(const char *word, Trie *trie)
{
    if (trie == NULL)
        return 0;
    if (*word == '\0')
    {
        return trie->is_word;
    }
    int index = *word - 'A';
    if (trie->figli[index] == NULL)
    {
        return -1;
    }
    return search_Trie(word + 1, trie->figli[index]);
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