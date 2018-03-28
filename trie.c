#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "trie.h"
#include "map.h"
#include "postings_list.h"


/*
 * Private functions, used only in here
 */
void import_text_map(trie_t* t, map_t* m);
void import_word(trie_t* t, char* word, int line_id);
trie_node_t* get_child(trie_node_t* node, char key);
trie_node_t* find_child(trie_node_t* node, char key);

/*
 * Create a full trie from a map
 */
trie_t* new_trie(map_t* m){
    trie_t* t = malloc(sizeof(trie_t));
    t->root = new_trie_node((char)-1, NULL);

    import_text_map(t, m); // import the words from the map

    return t;
}

/*
 * Given a trie and a line that has words,
 * import each of those words in that trie
 */
void import_text_map(trie_t* t, map_t* m){
    /*
     * Note - each line starts and ends with non-whitespace character
     */
    char c;
    char* tmp_word = NULL;
    size_t word_len = 0; // count the length of each word
    int start_index = 0; // first word starts at index 0
    bool new_word = false; // check if new word is found or not
    char* tmp_line = NULL;
    int i, j;

    for (i = 0; i < m->size; i++) {
        tmp_line = m->map[i];
        int len = strlen(tmp_line);
        for (j = 0; j < len; j++) {
            c = tmp_line[j];
            if (j == len-1)
                word_len++; // last char of last word.wouldnt increase otherwise
            if (isspace(c) || j == len-1) { // covering case of last word
                if (new_word == true) {
                    tmp_word = malloc(sizeof(char) * (word_len+1));

                    strncpy(tmp_word, &tmp_line[start_index], word_len);
                    tmp_word[word_len] = '\0';
                    import_word(t, tmp_word, i);

                    free(tmp_word);
                    tmp_word = NULL;
                    word_len = 0;
                    new_word = false;
                }
            }
            else {
                if (new_word == false) {
                    new_word = true;
                    start_index = j;
                }
                word_len++;
            }
        }
    }
}

void import_word(trie_t* t, char* word, int line_id){
    int len = strlen(word);
    trie_node_t* t_runner = t->root; // trie runner
    int i;
    for (i = 0; i < len; i++) {
        t_runner = get_child(t_runner, word[i]);
    }
    // the last child's postings list will store line_id with the right number
    plist_node_t* pl_runner = t_runner->plist; // postings list runner
    if (pl_runner == NULL) { // if this is the first ever occurence of this word
        pl_runner = new_plist_node(line_id, 0, NULL);
        t_runner->plist = pl_runner; // the new first node
        t_runner->plist_size++; // the list now has 1 node, the first one
    }
    else { // else, find the list node with id `line_id` (or create it)
        plist_node_t* prev = NULL;
        while (pl_runner != NULL) {
            if (pl_runner->id == line_id) // found the node with `line_id`
                break;

            else if (pl_runner->id > line_id) { // no node with `line_id` found
                plist_node_t* new_node = new_plist_node(line_id, 0, pl_runner);
                t_runner->plist_size++; // the list now has +1 node

                if (prev == NULL) // if there was only 1 node
                    t_runner->plist = new_node;

                else // case where we put the new node somewhere in the middle
                    prev->next = new_node;

                pl_runner = new_node;
                break;
            }
            else { // still no node with `line_id` (plist->id < line_id)
                if (pl_runner->next == NULL) { // add at end of list
                    plist_node_t* new_node = new_plist_node(line_id, 0, NULL);
                    t_runner->plist_size++; // the list now has +1 node

                    pl_runner->next = new_node;
                    pl_runner = new_node;
                    break;
                }
                else { // keep searching
                    prev = pl_runner;
                    pl_runner = pl_runner->next;
                }
            }
        }
    }
    pl_runner->num++;
}

/*
 * Given a node and a key, return the child of this node that has
 * this key. If none do, make a new one with that key and set it
 * as this node's child, so it can be found if searched later.
 */
trie_node_t* get_child(trie_node_t* node, char key){
    trie_node_t* runner = node->child;

    if (runner == NULL) { // if this node has no child node
        runner = new_trie_node(key, NULL);

        node->child = runner;
        return runner;
    }

    trie_node_t* prev = NULL; // the node before `runner`

    while (runner != NULL) {
        if (runner->key == key) // found it
            return runner;

        else if (runner->key > key) { // there wasn't a node with this key
            trie_node_t* new_node = new_trie_node(key, runner);

            if (prev == NULL) // if there was only 1 node
                node->child = new_node;

            else // case where we put the new node somewhere in the middle
                prev->next = new_node;

            return new_node;
        }
        else { // runner->key < key
            if(runner->next == NULL) { // add at end of list
                trie_node_t* new_node = new_trie_node(key, NULL);

                runner->next = new_node;
                return new_node;
            }
            else {
                prev = runner;
                runner = runner->next;
            }
        }
    }

    return NULL; // shouldn't reach this, something must have gone wrong
}

/*
 * Similar to get_child(), but if there is no child
 * node with that key, simply return NULL.
 */
trie_node_t* find_child(trie_node_t* node, char key){
    trie_node_t* runner = node->child;

    if (runner == NULL) // if this node has no child node
        return NULL;

    while (runner != NULL) {
        if (runner->key == key) // found it
            return runner;

        else if (runner->key > key) // there wasn't a node with this key
            return NULL;

        else { // runner->key < key
            if(runner->next == NULL) // end of list
                return NULL;

            else
                runner = runner->next;
        }
    }

    return NULL;
}

trie_node_t* new_trie_node(int key, trie_node_t* next){
    trie_node_t* new_node = malloc(sizeof(trie_node_t));
    new_node->key = key;
    new_node->next = next;
    new_node->child = NULL;
    new_node->plist = NULL;
    new_node->plist_size = 0;
    return new_node;
}

void rec_destroy_trie(trie_node_t* node){
    if (node == NULL)
        return;

    rec_destroy_trie(node->child);
    rec_destroy_trie(node->next);
    destroy_pl(node->plist);

    free(node);
}

void destroy_trie(trie_t* t){
    if (t == NULL)
        return;

    rec_destroy_trie(t->root);
    free(t);
}

/*
 * Return the number of occurrences of the word in that line.
 */
int find_occurrences(trie_t* t, int line_id, char* word){
    int i, len;
    len = strlen(word);
    trie_node_t* t_runner = t->root;
    for (i = 0; i < len; i++) {
        t_runner = find_child(t_runner, word[i]);
        if (t_runner == NULL)
            return 0;

    }

    plist_node_t* pl_runner = t_runner->plist;
    while (pl_runner != NULL && pl_runner->id != line_id)
        pl_runner = pl_runner->next;

    if (pl_runner == NULL)
        return 0;

    return pl_runner->num;
}

/*
 * Recursive print document frequency vector: Print how
 * many times each word of the tree is shown in the text.
 */
void rec_print_dfv(trie_node_t* node, char** bufptr){
    char* buffer = bufptr[0];
    int len = strlen(buffer);
    if (node->key == (char)-1) { // is trie root
        if (node->child != NULL)
            strcpy(buffer, &(node->child->key));
        else
            buffer[0] = '\0';

    }
    else
        buffer[len-1] = node->key; // put char at the end of the buffer-word

    if (node->plist != NULL) // has a postings list
        printf("%s %d\n", buffer, node->plist_size);

    if (node->child != NULL) { // has a child
        buffer[len] = 'a'; // temporarily put a char that'll be replaced
        buffer[len+1] = '\0'; // now 'a' will be replaced
        rec_print_dfv(node->child, bufptr);
        buffer[len] = '\0'; // resize buffer length back to this node's size
    }

    if (node->next != NULL)
        rec_print_dfv(node->next, bufptr);

    else { // decrement buffer length by 1 ONLY if at the end of list
        len = strlen(buffer);
        if (len > 0) // if this is root, buffer[-1] shouldn't be accessed
            buffer[len-1] = '\0';

    }
}

/*
 * If the word exists in the file,
 * print in how many lines it is.
 */
int get_df_num(trie_t* t, char* word){
    int i, len;
    len = strlen(word);
    trie_node_t* t_runner = t->root;
    for (i = 0; i < len; i++) {
        t_runner = find_child(t_runner, word[i]);
        if (t_runner == NULL)
            return -1; // word not in tree

    }

    return t_runner->plist_size;
}

/*
 * Given a number `num` of words, return an array of int
 * which has saved all the different ids of lines that
 * contain any of the words.
 * The array will save at index 0 how many elements there are after it.
 * ex.: {3, 15, 356, 663}
 */
int* in_which_lines(trie_t* t, int num, char** words){
    int* ids = malloc(sizeof(int));
    ids[0] = 0; // now there are 0 ids after ids[0]

    int i;
    for (i = 0; i < num; i++) { // for every word
        trie_node_t* t_runner = t->root;
        int j, len = strlen(words[i]);
        for (j = 0; j < len; j++) { // get the word's postings list if it has
            t_runner = find_child(t_runner, words[i][j]);
            if (t_runner == NULL)
                break; // word doesnt exist, stop searching it

        }
        if (t_runner == NULL) // if word doesn't exist, go to next one
            continue;

        plist_node_t* pl_runner = t_runner->plist;
        int found;
        while (pl_runner != NULL) { // make sure array has all ids of this word
            found = 0;
            int k;
            for (k = 0; k < ids[0]; k++) { // check all already saved ids
                if (pl_runner->id == ids[k+1]) {
                    found++;
                    // break; // if id was already saved, break and check next id
                }
            }
            if (found == 0) { // if it wasn't already found, save it
                ids[0] += 1;
                int* new_ids = malloc(sizeof(int) * (ids[0]+1));
                memcpy(new_ids, ids, ids[0] * sizeof(int));
                free(ids);
                ids = new_ids;
                ids[ids[0]] = pl_runner->id; // ids[ids[0]] == array's last id
            }

            pl_runner = pl_runner->next;
        }
    }

    return ids;
}