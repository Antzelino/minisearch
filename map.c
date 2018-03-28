#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "map.h"

/*
 * Private functions, used only in here
 */
char* remove_line_id(char* src);

map_t* new_map(FILE* fp){
    int size = count_lines(fp);
    if (size < 0)
        return NULL;

    map_t* m = malloc(sizeof(map_t));
    m->map = malloc(sizeof(char*) * size);
    m->dl = malloc(sizeof(int) * size);
    m->size = size;
    // m->avgdl is set at map_lines()
    return m;
}

/*
 * Make sure a string is consisting only
 * of numbers and it's positive.
 */
int is_number(const char* str){
    int i = 0;
    while (str[i] != '\0') {
        if (str[i] < '0' || str[i] > '9')
            return -1; // contains a non-numeric character

        i++;
    }
    return 1;
}

int count_lines(FILE* fp){
    /*
     * The pattern: The id of each line is exactly the same
     * number as the number of newlines that have been read.
     * So if they are equal every time, then the ids are correct.
     */
    int nl_count = 0; // will count '\n' then set `line_count`
    int ch;
    char id_str[128]; // suppose no number will have more than 128 digits
    int id = -1;
    /*
     * Need a way to know when I read the first line, or else I can't
     * read and verify the first id, because I only check the id if
     * the previous character I read was a newline. So if id == -1
     * I should check the first string I read, because it should be
     * a valid line id.
     */

    while (!feof(fp)) {
        ch = fgetc(fp);

        if(ch == '\n' || id == -1) {
            while (isspace(ch)) // keep going until non-whitespace
                ch = fgetc(fp);

            fseek(fp, -1L, SEEK_CUR); // go back 1 Byte to read whole word

            fscanf(fp, "%s", id_str); // read line id (first word)

            if (id != -1)
                nl_count++;

            if (is_number(id_str) < 0) {
                printf("Error: %s not an acceptable line id\n", id_str);
                return -1;
            }

            else if (sscanf(id_str, "%d", &id) != 1) { // parse id
                perror("sscanf");
                return -1;
            }
            if (feof(fp)) // just in case the file ends with a newline
                break;

            if (id != nl_count) { // check all line ids
                printf("Error: Wrong id at line %d.\n", nl_count);
                return -1;
            }
        }
    }
    rewind(fp);
    /*
     * The way the nl_count was calculated will help to
     * malloc only useful lines. If the file ends with one
     * or more '\n', it won't count as another line to
     * save in memory, even if there is some whitespace in it.
     */
    return nl_count;
}

int count_words(char* str){
    int len, i, count = 0;
    len = strlen(str);

    /*
     * One more word in the string means a non-space character is
     * read now and the next one is a space character (or '\0').
     */
    for (i = 0; i < len; i++)
        if (!isspace(str[i]) && str[i] != '\0' && (isspace(str[i+1]) || str[i+1] == '\0'))
            count++;

    return count;
}

void map_lines(map_t* m, FILE* fp){
    /*
     * According to getline(3) manpage we do this:
     * If first arg is set to NULL and 2nd arg is set to 0 before the
     * call, then getline() will malloc() a buffer for storing the line.
     */
    char* tmp_line = NULL;
    size_t n = 0;
    int len, i;
    m->avgdl = 0.0; // initialize running average with 0
    for (i = 0; i < m->size; i++) {
        while(isspace(fgetc(fp))) // keep going until non-whitespace
            ; // doesn't matter if there's whitespace between lines!

        fseek(fp, -1L, SEEK_CUR); // go back 1 Byte to read whole word

        getline(&tmp_line, &n, fp); // does a malloc for tmp_line if == NULL

        len = strlen(tmp_line);
        /*
         * The size of the pointer won't actually change, but
         * after this function, we will free this pointer and
         * make a new one without leading whitespace either.
         */
        while(isspace(tmp_line[len-1])){
            tmp_line[len-1] = '\0'; // "trim" trailing whitespace
            len--;
        }
        m->map[i] = remove_line_id(tmp_line); // also trims leading whitespace

        m->dl[i] = count_words(m->map[i]); // set document length of this line
        m->avgdl = (m->avgdl + (m->dl[i]/(i+1.0))) * ((i+1.0)/(i+2.0));

        tmp_line = NULL; // setting to NULL because of how getline works
        n = 0; // same as previous line
    }
}

void destroy_map(map_t* m){
    int i;
    for (i = 0; i < m->size; i++)
        free(m->map[i]);
    free(m->map);
    free(m->dl);
    free(m);
}

char* remove_line_id(char* src){
    int i = 0;
    while(isspace(src[i]))
        i++; // go through the supposed leading whitespace
    while (!isspace(src[i]))
        i++; // go through the line id
    while(isspace(src[i]))
        i++; // go throught the whitespace after the line id

    // Get the line from the first word to the last
    int len = strlen(&src[i]);
    char* without_id = malloc(sizeof(char) * (len+1));
    strcpy(without_id, &src[i]);
    free(src);
    return without_id;
}