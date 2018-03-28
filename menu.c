#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "menu.h"
#include "trie.h"
#include "map.h"

#define LCHLD(x) (2*x+1)
#define RCHLD(x) (LCHLD(x)+1)

/*
 * From mathematics: log_b(x) = log_a(x) / log_a(b)
 *
 * Cost to calculate log_10(any number) 100mil times:
 * With log10(x) :          ~3.285 s
 * With log(x) * base_10 :  ~2.565 s [in C log(double) is log_e(number)]
 *
 * With base_10 we can calculate log_10
 * of a number about ~1.28 times faster.
 */
const double base_10 = 0.4342944819; // = 1.0 / ln(10.0)

// Declaration of private functions
int count_digits(int num);
void heapify2(int idx, int* id_arr, double* sc_arr, int size);
void search(int argc, char** argv, trie_t* t, map_t* m);
void print_menu();
double bm25(trie_t* t, map_t* m, int line_id, int query_size, char** query);


/*
 * Takes integer, returns number
 * of digits of that integer.
 */
int count_digits(int num) {
    int ret = 1;
    int testnum = 10;

    while (num >= testnum) {
        ret++;
        testnum *= 10;
    }
    return ret;
}

/*
 * Get id array and score array and
 * max-heapify that size by the score.
 */
void heapify2(int idx, int* id_arr, double* sc_arr, int size){
    if (idx < 0)
        return;

    int tmp_max = idx;
    int l = LCHLD(idx);
    int r = RCHLD(idx);
    if (l < size && sc_arr[idx] < sc_arr[l])
        tmp_max = l;

    if (r < size && sc_arr[tmp_max] < sc_arr[r])
        tmp_max = r;

    if (tmp_max != idx) { // swap idx-th with tmp_max-th
        double dtmp = sc_arr[idx]; // swap in sc_arr array
        sc_arr[idx] = sc_arr[tmp_max];
        sc_arr[tmp_max] = dtmp;
        int itmp = id_arr[idx]; // swap in id_arr array
        id_arr[idx] = id_arr[tmp_max];
        id_arr[tmp_max] = itmp;
        heapify2(tmp_max, id_arr, sc_arr, size);
    }
}

void search(int argc, char** argv, trie_t* t, map_t* m){
    if (argc < 2) {
        printf("%s: Wrong number of arguments.\n", argv[0]);
        return;
    }
    else if (argc > 11)
        argc = 11; // use only up to the first 10 words for /search

    int* ids = in_which_lines(t, argc-1, &argv[1]); // lines that have the words
    int ids_size = ids[0]; // ids_size is how many ids there are after ids[0]

    int i;

    if (ids_size == 0) {
        printf("%s: word", argv[0]);
        if (argc > 2)
            printf("s");

        printf(" not found\n");
        free(ids);
        return;
    }
    ids = &ids[1]; // will revert back to normal exactly before we free this

    double* scores = malloc(sizeof(double) * ids_size);

    for (i = 0; i < ids_size; i++) // calculate score for each line
        scores[i] = bm25(t, m, ids[i], argc-1, &argv[1]);

    if (ids_size % 2 == 0) // ids_size of array is even number
        i = (ids_size-2)/2;
    else // ids_size of array is odd number
        i = (ids_size-1)/2 - 1;

    while (i >= 0)
        heapify2(i--, ids, scores, ids_size); // i is index of last nonleaf node

    int k_tmp = k;
    if (k_tmp > ids_size)
        k_tmp = ids_size-1;

    for (i = 0; i < k_tmp; i++) { // heapsort k_tmp elements
        int size = ids_size-i;
        // 1. "Delete" root node (max), put it at index [size-1]
        double dtmp = scores[0]; // swap in scores array
        scores[0] = scores[size-1];
        scores[size-1] = dtmp;
        int itmp = ids[0]; // swap in ids array
        ids[0] = ids[size-1];
        ids[size-1] = itmp;

        // 2. Heapify on root the heap with size of size-1
        heapify2(0, ids, scores, size-1);
    }

    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int cols = w.ws_col; // number of columns on terminal

    //  max length of the number of the counter
    int counter_max_len = count_digits(k_tmp);
    // max length of the number of the line id
    int id_max_len = count_digits(m->size-1);
    // max length of integer part of the score
    int int_float_max_len = count_digits((int) fabs(scores[ids_size-1]));

    for (i = 0; i < k_tmp; i++) { // now from last to last-k_tmp are sorted
        int line_id = ids[ids_size-i-1]; // the line id to print
        double score = scores[ids_size-i-1]; // the score of that line id

        int linelen = strlen(m->map[line_id]); // length of the map's line
        char* full_line = malloc(sizeof(char) * (linelen + 1));
        strcpy(full_line, m->map[line_id]);

        // length of characters printed before the map's line
        int infolen = printf("%-*d.(%*d)[% #*.4f] ",
                        counter_max_len, // char len of counter
                        i+1,
                        id_max_len, // char len of line id
                        line_id,
                        int_float_max_len, // char len of integer part of score
                        score
        );

        // part of the full line w/ size at cols characters (with previous str)
        char* line_part = malloc(sizeof(char)*(cols-infolen+1));
        int toprint = linelen; // characters left to print of the full_line
        int maxpartlen = cols - infolen; // maximum number of chars in line_part
        while (toprint > 0) {
            int offset = 0;
            int lastch = linelen-toprint+maxpartlen-1;
            if (!isspace(full_line[lastch])
            &&
            lastch+1 < linelen
            &&
            !isspace(full_line[lastch+1]))
                while (!isspace(full_line[lastch-offset]))
                    offset++; // so as to not cut a word

            strncpy(line_part, &full_line[linelen-toprint], maxpartlen-offset);
            line_part[maxpartlen-offset] = '\0'; // NULL-terminate it

            if (toprint != linelen) // if it's not the first line_part
                printf("%*c", infolen, ' '); // print the spaces

            printf("%s\n", line_part); // print next line_part

            // now print the line that underlines words from query
            int j;
            for (j = 1; j < argc; j++) { // for each word in query
                // argv[j] is the word of the query
                char* word_occ = strstr(line_part, argv[j]);
                int arglen = strlen(argv[j]);

                while(word_occ != NULL) { // find each occurence of word in it
                    int l = 0;

                    if ((word_occ == line_part || isspace(word_occ[-1]))
                    &&
                    (word_occ[arglen] == '\0' || isspace(word_occ[arglen]))) {
                        while (!isspace(word_occ[l]) && word_occ[l] != '\0')
                            word_occ[l++] = '^'; // replace word with '^' char

                    }
                    else
                        while (!isspace(word_occ[l]) && word_occ[l] != '\0')
                            l++;

                    word_occ = strstr(&word_occ[l], argv[j]); // keep searching
                }
            }
            // now replace everything that's non-space and not '^' with space
            j = 0;
            int part_len = strlen(line_part);
            while (j < part_len) {
                if (!isspace(line_part[j]) && line_part[j] != '^')
                    line_part[j] = ' ';

                j++;
            }
            printf("%*c", infolen, ' '); // first, print the spaces under info
            printf("%s\n", line_part); // then print "underline" of query words
            toprint -= (maxpartlen-offset);
        }
        free(line_part);
        free(full_line);
        printf("\n");
    }

    ids = &ids[-1]; // revert back to position of actual first element of ids
    free(scores);
    free(ids);
}

void df(int argc, char** argv, trie_t* t){
    if (argc > 2) {
        printf("%s: Wrong number of arguments.\n", argv[0]);
        return;
    }

    if (argc == 1) {
        char** bufptr = malloc(sizeof(char*));
        bufptr[0] = malloc(sizeof(char) * BUFFERSIZE);
        rec_print_dfv(t->root, bufptr); // recursive df to all nodes
        free(bufptr[0]);
        free(bufptr);
    }
    else {
        int n = get_df_num(t, argv[1]);
        printf("%s %d\n", argv[1], n);
    }
}

void tf(int argc, char** argv, trie_t* t, map_t* m){
    if (argc != 3) {
        printf("%s: Wrong number of arguments.\n", argv[0]);
        return;
    }

    int id;
    char* word = argv[2];
    if (is_number(argv[1]) < 0) {
        printf("%s: Wrong second argument.\n", argv[0]);
        return;
    }
    else {
        if (sscanf(argv[1], "%d", &id) != 1) {
            printf("%s: Couldn't parse second argument.\n", argv[0]);
            return;
        }
        else if (id >= m->size) {
            printf("%s: id too large. (max is %d)\n", argv[0], m->size-1);
            return;
        }
    }

    int n = find_occurrences(t, id, word);
    printf("%d %s %d\n", id, word, n);
}

void print_line(int argc, char** argv, map_t* m){
    if (argc != 2) {
        printf("%s: Wrong number of arguments.\n", argv[0]);
        return;
    }

    int id;
    if (sscanf(argv[1], "%d", &id) != 1) {
        printf("%s: Error in arguments.\n", argv[0]);
        return;
    }
    else if (id < 0 || id >= m->size) {
        printf("%s: id not in range [0 - %d].\n", argv[0], m->size-1);
        return;
    }

    printf("%s\n", m->map[id]);
}

/*
 * Returns < 0 if user wants to exit
 */
int parse_and_exec(int argc, char** argv, trie_t* t, map_t* m){
    if (strcmp(argv[0], "/help") == 0)
        print_menu();

    else if (strcmp(argv[0], "/clear") == 0)
        printf("\e[1;1H\e[2J");

    else if (strcmp(argv[0], "/search") == 0)
        search(argc, argv, t, m);

    else if (strcmp(argv[0], "/df") == 0)
        df(argc, argv, t);

    else if (strcmp(argv[0], "/tf") == 0)
        tf(argc, argv, t, m);

    else if (strcmp(argv[0], "/println") == 0)
        print_line(argc, argv, m);

    else if (strcmp(argv[0], "/exit") == 0)
        return -1;

    else
        printf("%s: Command not found.\n", argv[0]);

    return 0;
}

void menu_loop(trie_t* t, map_t* m){
    char* b_1 = "\e[1m";   // enable bold
    char* b_0 = "\e[0m";   // disable bold

    char *line = NULL;
    size_t bufsize = 0;
    while (1) {
        printf("%sminisearch> %s", b_1, b_0);
        getline(&line, &bufsize, stdin);

        int argc = count_words(line);

        if (argc <= 0){ // no need to stay in this loop, continue to the next
            free(line);
            line = NULL;
            bufsize = 0;
            continue;
        }

        char** argv = malloc(sizeof(char*) * argc);
        char* delims = " \t\n\f\r\v"; // some help from isspace manpage

        if (argv != NULL)
            argv[0] = strtok(line, delims);

        int i;
        for (i = 1; i < argc; i++)
            argv[i] = strtok(NULL, delims);

        int ret = parse_and_exec(argc, argv, t, m);

        free(argv);
        free(line);
        line = NULL;
        bufsize = 0;
        if (ret < 0)
            break;

    }
}

void print_menu(){
    /*
     * Escape sequences for BASH
     */
    char* ul_1 = "\e[4m";  // enable underline
    char* ul_0 = "\e[24m"; // disable underline
    char* b_1 = "\e[1m";   // enable bold
    char* b_0 = "\e[0m";   // disable bold

    printf("Commands:\n");
    printf("%s/help\n", b_1);
    printf("/clear\n");
    printf("/search%s %sq1%s [%sq2%s %sq3%s ... %sq10%s]\n",
        b_0, ul_1, ul_0, ul_1, ul_0, ul_1, ul_0, ul_1, ul_0);
    printf("%s/df%s [%sword%s]\n", b_1, b_0, ul_1, ul_0);
    printf("%s/tf%s %sid%s %sword%s\n", b_1, b_0, ul_1, ul_0, ul_1, ul_0);
    printf("%s/println%s %sid%s\n", b_1, b_0, ul_1, ul_0);
    printf("%s/exit%s\n", b_1, b_0);
}

double bm25(trie_t* t, map_t* m, int line_id, int query_size, char** query){
    double sum = 0.0; // final score, after the calculations
    double term_freq; // term frequency of query in a line
    double D; // number of words in line
    double avgdl = m->avgdl; // average document length in map/text
    double idf; // inverse doc freq
    double fraction; // the fraction after idf
    double N = (double) m->size; // number of lines in map/text
    double n_q; // number of lines that contain query word
    double k1 = 1.2;
    double b = 0.75;

    int i;
    for (i = 0; i < query_size; i++) {
        n_q = get_df_num(t, query[i]);
        if (n_q <= 0)
            continue; // if n(qi) is 0 then f(qi,D) is 0 for any D

        term_freq = find_occurrences(t, line_id, query[i]);
        if (term_freq == 0)
            continue; // if f(qi,D) is 0 then it will add 0 to sum, anyway

        idf = log((N-n_q+0.5)/(n_q+0.5)) * base_10;
        D = count_words(m->map[line_id]);
        fraction = term_freq*(k1+1.0)/(term_freq+(k1*(1.0-b+(b*D/avgdl))));

        sum += (idf * fraction);
    }

    return sum;
}
