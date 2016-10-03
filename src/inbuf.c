/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "grammar.h"
#include "inbuf.h"
#include "fatal.h"

/**
 * allocate an inbuf for g input (takes from freelist if possible)
 *
 * @param IN context
 */
inbuf_t * new_inbuf(input_t *IN)
{
    inbuf_t *inbuf, *next;
    int i;

    if (!IN->free_inbuf_list) {    // if no inbufs in free_inbuf_list

        IN->free_inbuf_list = malloc(INBUFALLOCNUM * sizeof(inbuf_t));
        if (!IN->free_inbuf_list) 
            fatal_perror("Error - malloc(): ");
        IN->stat_inbufmalloc++;

        next = IN->free_inbuf_list;    // link the new inbufs into free_inbuf_list
        i = INBUFALLOCNUM;
        while (i--) {
            inbuf = next++;
            inbuf->next = next;
        }
        inbuf->next = NULL;    // terminate last inbuf

    }
    inbuf = IN->free_inbuf_list;    // use first inbuf from free_inbuf_list
    IN->free_inbuf_list = inbuf->next;    // update list to point to next available

    inbuf->next = NULL;
    inbuf->refs = 0;
    inbuf->end_of_buf = '\0';    // parse() sees this null like an EOF

    IN->stat_inbufnow++;    // stats
    if (IN->stat_inbufnow > IN->stat_inbufmax) {
        IN->stat_inbufmax = IN->stat_inbufnow;
    }
    return inbuf;
}

/**
 * free an inbuf (not really freed, maintains freelist for reuse)
 *
 * @param IN context
 * @param inbuf
 */
void free_inbuf(input_t * IN, inbuf_t * inbuf)
{
    assert(inbuf);

    // insert inbuf into inbuf_freelist
    inbuf->next = IN->free_inbuf_list;
    IN->free_inbuf_list = inbuf;

    IN->stat_inbufnow--;    // stats
}
