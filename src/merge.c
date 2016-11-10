/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "thread.h"
#include "merge.h"

/**
 * merge old and new key lists
 *
 * @param LIST 
 * @param key - the key (and value) to be merged
 * @param oldkey - the key (and value) currently in the tree
 */
void merge_key(LIST_t *LIST, elem_t **key, elem_t *oldkey)
{

// FIXME

    oldkey->refs++;
    free_list(LIST, *key);
    *key = oldkey;
}
