/*
    Author: Nithin Nakka
    Email: nithin.nakka@gmail.com
*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "indexer.h"

int main()
{
    indexer_t *indexer;

    indexer = indexer_create(1, 100);
        print_indexer(indexer);
    for (int i = 1; i < 10; ++i) {
        PRINT_TRACE("Allocating %d indices", i);
        index_range_alloc(indexer, i);
        print_indexer(indexer);
    }
    PRINT_TRACE("Allocating 55 indices");
    index_range_alloc(indexer, 29);
    print_indexer(indexer);
    index_range_alloc(indexer, 26);
    print_indexer(indexer);

    PRINT_TRACE("Deallocing 4 now");
    index_range_dealloc(indexer, 4);
    print_indexer(indexer);
    PRINT_TRACE("Deallocing 11 now");
    index_range_dealloc(indexer, 11);
    print_indexer(indexer);
    PRINT_TRACE("Deallocing 22 now");
    index_range_dealloc(indexer, 22);
    print_indexer(indexer);
    PRINT_TRACE("Deallocing 29 now");
    index_range_dealloc(indexer, 29);
    print_indexer(indexer);
    PRINT_TRACE("Deallocing 16 now");
    index_range_dealloc(indexer, 16);
    print_indexer(indexer);
    PRINT_TRACE("Deallocing 1 now");
    index_range_dealloc(indexer, 1);
    print_indexer(indexer);
    PRINT_TRACE("Deallocing 75 now");
    index_range_dealloc(indexer, 75);
    print_indexer(indexer);

    PRINT_TRACE("Allocating 26 indices");
    index_range_alloc(indexer, 26);
    print_indexer(indexer);
    index_range_alloc(indexer, 26);
    print_indexer(indexer);
    PRINT_TRACE("Deallocing 75 now");
    index_range_dealloc(indexer, 75);
    print_indexer(indexer);
    PRINT_TRACE("Deallocing 11 now");
    index_range_dealloc(indexer, 11);
    print_indexer(indexer);
    PRINT_TRACE("Deallocing 7 now");
    index_range_dealloc(indexer, 7);
    print_indexer(indexer);
    return 0;
}
