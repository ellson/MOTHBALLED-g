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
 * @param oldkey 
 * @param newkey 
 */
void merge(LIST_t *LIST, elem_t * oldkey, elem_t * newkey)
{

// FIXME

    oldkey->refs--;
    newkey->refs++;
}
