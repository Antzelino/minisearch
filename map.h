#ifndef MAP_H
#define MAP_H

#include <stdlib.h>


typedef struct map_t{
    char** map;
    int size;
    int* dl;
    double avgdl;
} map_t;

map_t* new_map(FILE* fp);
int count_lines(FILE* fp);
int count_words(char* str);
void map_lines(map_t* map, FILE* fp);
void destroy_map(map_t* map);
int is_number(const char* str);

#endif // MAP_H
