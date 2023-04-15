#ifndef __INDEXER_H__
#define __INDEXER_H__

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include "avl.h"

#define UNIMPLEMENTED \
    do { \
        fprintf(stderr, "%s:%d: %s is not implemented yet\n", \
                __FILE__, __LINE__, __func__); \
        abort(); \
    } while(0)

#define PRINT_ERROR(fmt, args...) \
    do { \
        fprintf(stderr, "%s:%d: %s ERROR: "fmt"\n", \
                __FILE__, __LINE__, __func__, ##args); \
    } while(0)

#define PRINT_TRACE(fmt, args...) \
    do { \
        fprintf(stdout, "%s:%d: %s TRACE: "fmt"\n", \
                __FILE__, __LINE__, __func__, ##args); \
    } while(0)

#define PRINT_INFO(fmt, args...) \
    do { \
        fprintf(stdout, "%s:%d: %s INFO: "fmt"\n", \
                __FILE__, __LINE__, __func__, ##args); \
    } while(0)

typedef struct {
    int start;
    int range;
    TREE *alloced_blocks_start;
    TREE *free_blocks_start;
    TREE *free_blocks_end;
    TREE *free_blocks_range;
} indexer_t;

indexer_t *indexer_create(int start, int range);
int index_range_alloc(indexer_t *indexer, int block_size);
void index_range_dealloc(indexer_t *indexer, int start_index);
void print_indexer(indexer_t *indexer);
bool print_block(void *data);

#define MAX_block_list_t 1024

typedef struct block_t_ {
    int start;
    int end;
    int range;
    struct block_t_ *next;
} block_t;

#endif // __INDEXER_H__
