/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "fatal.h"
#include "inbuf.h"

/**
 * allocate an inbuf for g input (takes from freelist if possible)
 *
 * @param INBUFS context
 */
inbuf_t * new_inbuf(INBUFS_t *INBUFS)
{
    inbuf_t *inbuf, *next;
    int i;

    if (!INBUFS->free_inbuf_list) {    // if no inbufs in free_inbuf_list

        INBUFS->free_inbuf_list = malloc(INBUFALLOCNUM * sizeof(inbuf_t));
        if (!INBUFS->free_inbuf_list) 
            fatal_perror("Error - malloc(): ");
        INBUFS->stat_inbufmalloc++;

        next = INBUFS->free_inbuf_list;    // link the new inbufs into free_inbuf_list
        i = INBUFALLOCNUM;
        while (i--) {
            inbuf = next++;
            inbuf->next = next;
        }
        inbuf->next = NULL;    // terminate last inbuf

    }
    inbuf = INBUFS->free_inbuf_list;    // use first inbuf from free_inbuf_list
    INBUFS->free_inbuf_list = inbuf->next;    // update list to point to next available

    inbuf->next = NULL;
    inbuf->refs = 0;
    inbuf->end_of_buf = '\0';    // parse() sees this null like an EOF

    INBUFS->stat_inbufnow++;    // stats
    if (INBUFS->stat_inbufnow > INBUFS->stat_inbufmax) {
        INBUFS->stat_inbufmax = INBUFS->stat_inbufnow;
    }
    return inbuf;
}

/**
 * free an inbuf (not really freed, maintains freelist for reuse)
 *
 * @param INBUFS context
 * @param inbuf
 */
void free_inbuf(INBUFS_t * INBUFS, inbuf_t * inbuf)
{
    assert(inbuf);

    // insert inbuf into inbuf_freelist
    inbuf->next = INBUFS->free_inbuf_list;
    INBUFS->free_inbuf_list = inbuf;

    INBUFS->stat_inbufnow--;    // stats
}
