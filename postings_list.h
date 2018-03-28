#ifndef POSTINGS_LIST_H
#define POSTINGS_LIST_H


typedef struct postings_list_node_t{
    int id; // id of line
    int num; // number of occurences in this line
    struct postings_list_node_t* next; // pointer to next node, if there is one
} plist_node_t;

plist_node_t* new_plist_node(int id, int num, plist_node_t* next);
void destroy_pl(plist_node_t* head);

#endif // POSTINGS_LIST_H
