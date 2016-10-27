/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "compare.h"

/**
 * merge old and new attributes
 *
 * @param new 
 * @param old 
 * @return merge
 */
elem_t * merge(elem_t * new, elem_t * old)
{

// FIXME

    old->refs--;
    new->refs++;
    return new;
}
