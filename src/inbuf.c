/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <errno.h>
#include <assert.h>

#include "libje_private.h"

/**
 * allocate an inbuf for g input (takes from freelist if possible)
 *
 * @param C context
 */
void new_inbuf(context_t *C)
{
    inbuf_t *inbuf, *next;
    int i;

    if (!C->free_inbuf_list) {    // if no inbufs in free_inbuf_list

        C->free_inbuf_list = malloc(INBUFALLOCNUM * sizeof(inbuf_t));
        if (!C->free_inbuf_list) {
            perror("Error - malloc(): ");
            exit(EXIT_FAILURE);
        }
        C->stat_inbufmalloc++;

        next = C->free_inbuf_list;    // link the new inbufs into free_inbuf_list
        i = INBUFALLOCNUM;
        while (i--) {
            inbuf = next++;
            inbuf->next = next;
        }
        inbuf->next = NULL;    // terminate last inbuf

    }
    inbuf = C->free_inbuf_list;    // use first inbuf from free_inbuf_list
    C->free_inbuf_list = inbuf->next;    // update list to point to next available

    inbuf->next = NULL;
    inbuf->refs = 0;
    inbuf->end_of_buf = '\0';    // parse() sees this null like an EOF

    C->stat_inbufnow++;    // stats
    if (C->stat_inbufnow > C->stat_inbufmax) {
        C->stat_inbufmax = C->stat_inbufnow;
    }
    C->inbuf = inbuf;
}

/**
 * free an inbuf (not really freed, maintains freelist for reuse)
 *
 * @param C context
 * @param inbuf
 */
void free_inbuf(context_t *C, inbuf_t * inbuf)
{
    assert(inbuf);

    // insert inbuf into inbuf_freelist
    inbuf->next = C->free_inbuf_list;
    C->free_inbuf_list = inbuf;

    C->stat_inbufnow--;    // stats
}
