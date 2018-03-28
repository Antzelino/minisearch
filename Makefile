# Anxhelino Mehmeti 1115201400233 Ergasia1 Makefile

OBJS = main.o map.o trie.o postings_list.o menu.o
SOURCES = main.c map.c trie.c postings_list.c menu.c
CC = gcc
OUT = minisearch
FLAGS = -Wall -g

$(OUT): $(OBJS)
	$(CC) $(FLAGS) $^ -o $@ -lm
	@echo -n "\nExecutable: ./minisearch\n"

main.o: main.c
	$(CC) $(FLAGS) -c $<

map.o: map.c map.h
	$(CC) $(FLAGS) -c $<

trie.o: trie.c trie.h
	$(CC) $(FLAGS) -c $<

postings_list.o: postings_list.c postings_list.h
	$(CC) $(FLAGS) -c $<

menu.o: menu.c menu.h
	$(CC) $(FLAGS) -c $<

clean:
	rm -f $(OUT) $(OBJS)
