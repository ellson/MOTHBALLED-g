/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "fatal.h"
#include "inbuf.h"

/**
 * allocate an inbuf for g input (takes from freelist if possible)
 *
 * @param INBUF context
 */
inbuf_t * new_inbuf(INBUF_t *INBUF)
{
    inbuf_t *inbuf, *next;
    int i;

    if (!INBUF->free_inbuf_list) {    // if no inbufs in free_inbuf_list

        INBUF->free_inbuf_list = malloc(INBUFALLOCNUM * sizeof(inbuf_t));
        if (!INBUF->free_inbuf_list) 
            fatal_perror("Error - malloc(): ");
        INBUF->stat_inbufmalloc++;

        next = INBUF->free_inbuf_list;    // link the new inbufs into free_inbuf_list
        i = INBUFALLOCNUM;
        while (i--) {
            inbuf = next++;
            inbuf->next = next;
        }
        inbuf->next = NULL;    // terminate last inbuf

    }
    inbuf = INBUF->free_inbuf_list;    // use first inbuf from free_inbuf_list
    INBUF->free_inbuf_list = inbuf->next;    // update list to point to next available

    inbuf->next = NULL;
    inbuf->refs = 0;
    inbuf->end_of_buf = '\0';    // parse() sees this null like an EOF

    INBUF->stat_inbufnow++;    // stats
    if (INBUF->stat_inbufnow > INBUF->stat_inbufmax) {
        INBUF->stat_inbufmax = INBUF->stat_inbufnow;
    }
    return inbuf;
}

/**
 * free an inbuf (not really freed, maintains freelist for reuse)
 *
 * @param INBUF context
 * @param inbuf
 */
void free_inbuf(INBUF_t * INBUF, inbuf_t * inbuf)
{
    assert(inbuf);

    // insert inbuf into inbuf_freelist
    inbuf->next = INBUF->free_inbuf_list;
    INBUF->free_inbuf_list = inbuf;

    INBUF->stat_inbufnow--;    // stats
}
