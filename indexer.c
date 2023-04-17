/*
    Author: Nithin Nakka
    Email: nithin.nakka@gmail.com
*/

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "indexer.h"

indexer_t *indexer_create(int start, int range)
{
    indexer_t *indexer = NULL;
    block_t *block = NULL;
    bool inserted = false;

    if (start <= 0 || range <= 0)
    {
        PRINT_ERROR("Invalid values provided for start and range");
        return NULL;
    }

    indexer = (indexer_t *)malloc(sizeof(indexer_t));
    if (indexer == NULL)
    {
        PRINT_ERROR("OOM (Out of memory) - Unable to allocate indexer.");
        return NULL;
    }
    memset(indexer, 0, sizeof(indexer_t));

    block = (block_t *)malloc(sizeof(block_t));
    if (block == NULL)
    {
        PRINT_ERROR("OOM (Out of memory) - Unable to allocate block.");
        free(indexer);
        return NULL;
    }
    memset(block, 0, sizeof(block_t));

    indexer->start = start;
    indexer->range = range;
    block->start = start;
    block->end = range;
    block->range = range;
    block->next = NULL;

    indexer->alloced_blocks_start = avl_tree_nodup_int(block_t, start);
    indexer->free_blocks_start = avl_tree_nodup_int(block_t, start);
    indexer->free_blocks_end = avl_tree_nodup_int(block_t, end);
    indexer->free_blocks_range = avl_tree_nodup_int(block_t, range);

    inserted = avl_insert(indexer->free_blocks_start, block);
    if (inserted) {
        inserted = avl_insert(indexer->free_blocks_end, block);
    }
    if (inserted) {
        inserted = avl_insert(indexer->free_blocks_range, block);
    }
    if (!inserted) {
        PRINT_ERROR("Unable to insert block to initial free_blocks_start");
        free(block);
        avl_free(indexer->alloced_blocks_start);
        avl_free(indexer->free_blocks_start);
        avl_free(indexer->free_blocks_end);
        avl_free(indexer->free_blocks_range);
        free(indexer);
        return NULL;
    }

    return indexer;
}

int index_range_alloc(indexer_t *indexer, int range)
{
    block_t *block = NULL;
    block_t *new_free_block = NULL;
    block_t *next_free_range_block = NULL;

    if (range <= 0)
    {
        PRINT_ERROR("Invalid range provided for allocation");
        return -1;
    }

    // find oldest free block with range that is >= range
    block = (block_t *) avl_locate_ge_int(indexer->free_blocks_range, range);
    if (block == NULL)
    {
        PRINT_ERROR("Unable to find a contiguous index block of range %d", range);
        return -1;
    }

    /* Consider different scenarios:
       Case 1: Free block range is same as requested range then
            a. Delete the block from the free blocks (all 3 avls)
            b. Insert the block in the alloced blocks 
        Case 2: Free block range is greater than requested range then
            a. Delete block from the free blocks (all 3 avls)
            b. Create a new free block
            c. Update new free block start and range fields
            d. Insert new free block into freed blocks (all 3 avls)
            e. Insert block with requested range in the alloced blocks */
    avl_remove_int(indexer->free_blocks_start, block->start);
    avl_remove_int(indexer->free_blocks_end, block->end);
    avl_remove_int(indexer->free_blocks_range, block->range);
    if (block->next != NULL) {
        // There are multiple blocks with the same range. Pop the first one
        // and re-insert the remaining into the range avl
        next_free_range_block = block->next;
        avl_insert(indexer->free_blocks_range, next_free_range_block);
    }

    const int tail_range = block->range - range;
    if (tail_range > 0)
    {
        new_free_block = (block_t *)malloc(sizeof(block_t));
        if (new_free_block == NULL)
        {
            PRINT_ERROR("OOM (Out of memory) - Unable to allocate new free block.");
            return -1;
        }
        memset(new_free_block, 0, sizeof(block_t));

        new_free_block->start = block->start + range;
        new_free_block->end = block->end;
        new_free_block->range = tail_range;

        avl_insert(indexer->free_blocks_start, new_free_block);
        avl_insert(indexer->free_blocks_end, new_free_block);
        next_free_range_block = avl_locate_int(indexer->free_blocks_range,
                                               new_free_block->range);
        new_free_block->next = next_free_range_block;
        avl_insert(indexer->free_blocks_range, new_free_block);
    }

    block->range = range;
    block->end = block->start + block->range - 1;

    avl_insert(indexer->alloced_blocks_start, block);

    return block->start;
}

void index_range_dealloc(indexer_t *indexer, int start)
{
    block_t *block = NULL;
    block_t *prev_block = NULL;
    block_t *next_block = NULL;
    block_t *curr_range_block = NULL;
    block_t *prev_range_block = NULL;
    block_t *next_range_block = NULL;

    if (start <= 0)
    {
        PRINT_ERROR("Invalid start index provided to free");
        return;
    }

    block = avl_remove_int(indexer->alloced_blocks_start, start);
    if (block == NULL)
    {
        PRINT_ERROR("Unable to find allocated index range block starting with %d", start);
        return;
    }
    // print_block((void *) block);

    // Handle the scenario when block->end is the end of the indexer range
    next_block = avl_locate_int(indexer->free_blocks_start, block->end + 1);
    if (next_block == NULL)
    {
        PRINT_INFO("No next block found to merge: %d", block->end + 1);
    }
    // print_block((void *) next_block);

    // Handle the case when block->start is the start of the indexer range
    prev_block = avl_locate_int(indexer->free_blocks_end, block->start - 1);
    if (prev_block == NULL)
    {
        PRINT_INFO("No prev block found to merge: %d", block->start - 1);
    }
    // print_block((void *) prev_block);

    if (next_block == NULL && prev_block == NULL)
    {
        PRINT_TRACE("No prev or next blocks");
        avl_insert(indexer->free_blocks_start, block);
        avl_insert(indexer->free_blocks_end, block);
        next_range_block = (block_t *)avl_remove_int(indexer->free_blocks_range,
                                                     block->range);
        block->next = next_range_block;
        avl_insert(indexer->free_blocks_range, block);
    }
    if (next_block != NULL)
    {
        PRINT_TRACE("Next block found");
        avl_remove_int(indexer->free_blocks_start, next_block->start);

        curr_range_block = (block_t *)avl_remove_int(indexer->free_blocks_range,
                                                     next_block->range);
        if ((curr_range_block->start == next_block->start) 
                && (curr_range_block->end == next_block->end))
        {
            if (curr_range_block->next != NULL)
            {
                avl_insert(indexer->free_blocks_range, curr_range_block->next);
            }
        } else {
            next_range_block = curr_range_block;
            while (next_range_block != NULL 
                    && (next_range_block->start == next_block->start) 
                    && (next_range_block->end == next_block->end))
            {
                prev_range_block = next_range_block;
                next_range_block = next_range_block->next;
            }

            if (next_range_block == NULL)
            {
                PRINT_ERROR("Did not find start and end match for given range");
                assert(0);
            }
            prev_range_block->next = next_range_block->next;
            avl_insert(indexer->free_blocks_range, curr_range_block);
        }

        next_block->start = block->start;
        next_block->range += block->range;
        avl_insert(indexer->free_blocks_start, next_block);

        next_range_block = (block_t *)avl_remove_int(indexer->free_blocks_range, 
                                                     next_block->range);
        next_block->next = next_range_block;
        avl_insert(indexer->free_blocks_range, next_block);

        free(block);
        block = next_block;
    }
    if (prev_block != NULL)
    {
        PRINT_TRACE("Prev block found");
        avl_remove_int(indexer->free_blocks_end, prev_block->end);

        if (next_block != NULL) {
            avl_remove_int(indexer->free_blocks_start, block->start);
            avl_remove_int(indexer->free_blocks_end, block->end);
            curr_range_block = (block_t *)avl_remove_int(indexer->free_blocks_range,
                                                         block->range);
            if ((curr_range_block->start == block->start) 
                    && (curr_range_block->end == block->end))
            {
                if (curr_range_block->next != NULL)
                {
                    avl_insert(indexer->free_blocks_range, curr_range_block->next);
                }
            } else {
                next_range_block = curr_range_block;
                while (next_range_block != NULL 
                        && (next_range_block->start == block->start) 
                        && (next_range_block->end == block->end))
                {
                    prev_range_block = next_range_block;
                    next_range_block = next_range_block->next;
                }

                if (next_range_block == NULL)
                {
                    PRINT_ERROR("Did not find start and end match for given range");
                    assert(0);
                }
                prev_range_block->next = next_range_block->next;
                avl_insert(indexer->free_blocks_range, curr_range_block);
            }
        }

        curr_range_block = (block_t *)avl_remove_int(indexer->free_blocks_range, 
                                                     prev_block->range);
        if ((curr_range_block->start == prev_block->start) 
                && (curr_range_block->end == prev_block->end))
        {
            if (curr_range_block->next != NULL)
            {
                avl_insert(indexer->free_blocks_range, curr_range_block->next);
            }
        } else {
            next_range_block = curr_range_block;
            while (next_range_block != NULL 
                    && (next_range_block->start == prev_block->start) 
                    && (next_range_block->end == prev_block->end))
            {
                prev_range_block = next_range_block;
                next_range_block = next_range_block->next;
            }

            if (next_range_block == NULL)
            {
                PRINT_ERROR("Did not find start and end match for given range");
                assert(0);
            }
            prev_range_block->next = next_range_block->next;
            avl_insert(indexer->free_blocks_range, curr_range_block);
        }

        prev_block->end = block->end;
        prev_block->range += block->range;
        avl_insert(indexer->free_blocks_end, prev_block);

        next_range_block = (block_t *)avl_remove_int(indexer->free_blocks_range,
                                                     prev_block->range);
        prev_block->next = next_range_block;
        avl_insert(indexer->free_blocks_range, prev_block);

        free(block);
    }
}

bool print_block(void *data)
{
    block_t *block = (block_t *) data;
    PRINT_INFO("\tPrinting block: %p", block);
    while (block != NULL) {
        PRINT_INFO("\t\tBlock->start: %d", block->start);
        PRINT_INFO("\t\tBlock->end: %d", block->end);
        PRINT_INFO("\t\tBlock->range: %d", block->range);
        PRINT_INFO("\t\tBlock->next: %p", block->next);
        block = block->next;
    }
}

void print_indexer(indexer_t *indexer)
{
    PRINT_INFO("Indexer start: %d", indexer->start);
    PRINT_INFO("Indexer range: %d", indexer->range);
    PRINT_INFO("\tIndexer alloced blocks start");
    avl_scan(indexer->alloced_blocks_start, print_block);
    PRINT_INFO("\tIndexer free blocks start");
    avl_scan(indexer->free_blocks_start, print_block);
    PRINT_INFO("\tIndexer free blocks end");
    avl_scan(indexer->free_blocks_end, print_block);
    PRINT_INFO("\tIndexer free blocks range");
    avl_scan(indexer->free_blocks_range, print_block);
}