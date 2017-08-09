/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "types.h"
#include "fatal.h"
#include "inbuf.h"

/**
 * allocate an inbuf for g input (takes from freelist if possible)
 *
 * @param INBUF context
 */
inbufelem_t * new_inbuf(INBUF_t *INBUF)
{
    inbufelem_t *inbuf, *nextinbuf;

    if (!INBUF->free_inbuf_list) {    // if no inbufs in free_inbuf_list

        INBUF->free_inbuf_list = malloc(INBUFALLOCNUM * sizeof(inbufelem_t));
        if (!INBUF->free_inbuf_list) 
            FATAL("malloc()");
        INBUF->stat_inbufmalloc++;

        nextinbuf = INBUF->free_inbuf_list;    // link the new inbufs into free_inbuf_list
        int i = INBUFALLOCNUM;
        while (i--) {
            inbuf = nextinbuf++;
            inbuf->nextinbuf = nextinbuf;
        }
        inbuf->nextinbuf = NULL;    // terminate last inbuf

    }
    inbuf = INBUF->free_inbuf_list;    // use first inbuf from free_inbuf_list
    INBUF->free_inbuf_list = inbuf->nextinbuf; // point to next available

    inbuf->nextinbuf = NULL;
    inbuf->refs = 0;

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
void free_inbuf(INBUF_t * INBUF, inbufelem_t * inbuf)
{
    assert(inbuf);

    // insert inbuf into inbuf_freelist
    inbuf->nextinbuf = INBUF->free_inbuf_list;
    INBUF->free_inbuf_list = inbuf;

    INBUF->stat_inbufnow--;    // stats
}
