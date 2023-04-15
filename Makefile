# CFLAGS=-Wall -Wextra -Werror -std=c11 -pedantic -ggdb
CFLAGS=-ggdb

indexer: main.c indexer.c indexer.h avl.c avl.h
	$(CC) $(CFLAGS) -o indexer main.c indexer.c avl.c
