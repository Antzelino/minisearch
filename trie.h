#ifndef TRIE_H
#define TRIE_H

#include "map.h"
#include "postings_list.h"

#define BUFFERSIZE 256 // Buffer/Stack to store each word to be printed in df


typedef struct trie_node_t{
    char key;
    struct trie_node_t* next;
    struct trie_node_t* child;
    plist_node_t* plist;
    int plist_size;
} trie_node_t;

typedef struct trie_t{
    trie_node_t* root;
} trie_t;

typedef enum {
    false,
    true
} bool;


trie_t* new_trie(map_t*  map);
trie_node_t* new_trie_node(int key, trie_node_t* next);
void destroy_trie(trie_t* t);
int find_occurrences(trie_t* t, int line_id, char* word);
void rec_print_dfv(trie_node_t* node, char** bufptr);
int get_df_num(trie_t* t, char* word);
int* in_which_lines(trie_t* t, int num, char** words);

#endif // TRIE_H
