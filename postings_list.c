#include <stdlib.h>
#include "postings_list.h"

plist_node_t* new_plist_node(int id, int num, plist_node_t* next){
    plist_node_t* new_node = malloc(sizeof(plist_node_t));

    new_node->id = id;
    new_node->num = num;
    new_node->next = next;
    return new_node;
}

void rec_destroy_pl(plist_node_t* node){
    if (node == NULL)
        return;

    rec_destroy_pl(node->next);
    free(node);
}

/*
 * Call this on head of list to destroy it all
 */
void destroy_pl(plist_node_t* head){
    rec_destroy_pl(head);
}