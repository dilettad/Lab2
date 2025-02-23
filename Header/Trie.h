#ifndef TRIE_H
#define TRIE_H
#define NUM_CHAR 26

typedef struct Trie{
    struct Trie *figli[NUM_CHAR];
    int is_word;
} Trie;

Trie *create_node();

int insert_Trie(Trie *trie,  char *word);

int search_Trie( char *word, Trie *trie);

void Print_Trie(Trie *trie, char *buffer, int depth);

#endif