#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "map.h"
#include "trie.h"
#include "menu.h"


int k;

/*
 * Escape sequences for BASH
 */
char* ul_1 = "\e[4m";  // enable underline
char* ul_0 = "\e[24m"; // disable underline
char* b_1 = "\e[1m";   // enable bold
char* b_0 = "\e[0m";   // disable bold

/*
 * Print a synopsis of how to run the program
 */
void print_synopsis(){
    printf("Synopsis:\n");
    printf("./minisearch %s-i%s %sdocfile%s", b_1, b_0, ul_1, ul_0);
    printf(" [%s-k%s %sK%s]\n", b_1, b_0, ul_1, ul_0);
}

/*
 * Allocate enough memory for a char* string to be
 * at most, as large as the input char* string.
 */
char* alloc_str(const char* str){
    return malloc((strlen(str)+1) * sizeof(char));
}

int main(int argc, char const *argv[]) {
    // Checking arguments
    if (argc != 5 && argc != 3) {
      printf("Error: Wrong number of arguments. ");
      print_synopsis();
      return 1;
    }

    // 0. Preparation - Flags and error checking
    k = 10;
    int i;
    int filename_index; // name of input file
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "-I") == 0)
            filename_index = ++i;

        else if (strcmp(argv[i], "-k") == 0 || strcmp(argv[i], "-K") == 0) {
            if (is_number(argv[++i]) < 0) {
                printf("Error: K argument not an acceptable number.\n");
                return 1;
            }
            else {
                if (sscanf(argv[i], "%d", &k) != 1) {
                    printf("Error in argument K\n");
                    return 1;
                }
                if (k < 1) {
                    printf("Error: K argument not an acceptable number.\n");
                    return 1;
                }
            }
        }
        else {
            printf("Error: Wrong argument in command line. ");
            print_synopsis();
            return 1;
        }
    }

    // 1. Map - Create dynamically allocated map of input docfile
    // 1.1 Count lines to find size of allocation
    FILE* fp = fopen(argv[filename_index], "r");
    if (fp == NULL) {
        perror("fopen");
        return 1;
    }


    // 1.2 Dynamically malloc the map's size
    printf("\e[1;1H\e[2J"); // Clear console
    printf("Loading map.....");
    fflush(stdout);
    map_t* m = new_map(fp);
    if (m == NULL) {
        printf("new_map: Error\n");
        return 1;
    }

    // 1.3 Dynamically map each line (lines don't have a static size)
    printf("\nMapping lines...");
    fflush(stdout);
    map_lines(m, fp);
    fclose(fp); // we have the map so we can close the file

    // 2. Trie - Create dynamically allocated Trie from the Map
    printf("\nCreating trie...");
    fflush(stdout);
    trie_t* t = new_trie(m);
    if (t == NULL) {
        perror("new_trie");
        return 1;
    }
    // 3. Menu - Run CLI
    printf("\e[1;1H\e[2J"); // Clear console
    menu_loop(t, m);

    // 4. End - Destroy Trie and Map
    destroy_trie(t);
    destroy_map(m);
    return 0;
}
