/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "thread.h"
#include "merge.h"

/**
 * merge a new attrid which matches an existing attrib
 * by updating new to use the identical existing, and then freeing the duplicate
 *
 * @param LIST 
 * @param attrid - the new (duplicate) attrid
 * @param key    - the key with the original attrid
 */
void merge_attrid(LIST_t *LIST, elem_t **attrid, elem_t *key)
{
    key->refs++;
    free_list(LIST, *attrid);
    *attrid = key;
}

/**
 * merge ATTRIBUTEs for a NODE or EDGE which may have existing ATTRIBUTES
 *
 * @param LIST 
 * @param attributes - the new attributes
 * @param key        - the key with previous attributes
 */
void merge_attributes(LIST_t *LIST, elem_t **attributes, elem_t *key)
{
    //FIXME
}

/**
 * merge a new pattern act
 * by replacing old ATTRIBUTES with ATTRIBUTES from the new pattern act.
 *
 * @param LIST 
 * @param act - the new (replacement) pattern act
 * @param key - the key with the old pattern act
 */
void merge_pattern(LIST_t *LIST, elem_t **act, elem_t *key)
{
    // free old ATTRIBUTES and append new ATTRIBUTES
    free_list(LIST, key->u.l.first->u.l.next);
    key->u.l.first->u.l.next = ref_list(LIST, (*act)->u.l.first->u.l.next);
}
