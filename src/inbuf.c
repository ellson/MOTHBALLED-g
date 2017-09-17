/* vim:set shiftwidth=4 ts=8 expandtab: */

/*************************************************************************
 * Copyright (c) 2017 AT&T Intellectual Property
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: John Ellson <john.ellson@gmail.com>
 *************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "types.h"
#include "fatal.h"
#include "inbuf.h"

/**
 * allocate an inbuf for g input (takes from freelist if possible)
 *
 * @param INBUF thread context
 */
inbufelem_t * new_inbuf(INBUF_t *INBUF)
{
    PROC_INBUF_t *PROC_INBUF = INBUF->PROC_INBUF;
    inbufelem_t *inbuf, *nextinbuf;

    if (!PROC_INBUF->free_inbuf_list) {    // if no inbufs in free_inbuf_list

        PROC_INBUF->free_inbuf_list = malloc(INBUFALLOCNUM * sizeof(inbufelem_t));
        if (!PROC_INBUF->free_inbuf_list) 
            FATAL("malloc()");
        PROC_INBUF->stat_inbufmalloc++;

        nextinbuf = PROC_INBUF->free_inbuf_list;    // link the new inbufs into free_inbuf_list
        int i = INBUFALLOCNUM;
        while (i--) {
            inbuf = nextinbuf++;
            inbuf->u.nextinbuf = nextinbuf;
        }
        inbuf->u.nextinbuf = NULL;    // terminate last inbuf

    }
    inbuf = PROC_INBUF->free_inbuf_list;    // use first inbuf from free_inbuf_list
    PROC_INBUF->free_inbuf_list = inbuf->u.nextinbuf; // point to next available

    inbuf->u.nextinbuf = NULL;  // also initializes: inbuf->u.refs = 0;

    PROC_INBUF->stat_inbufnow++;    // stats
    if (PROC_INBUF->stat_inbufnow > PROC_INBUF->stat_inbufmax) {
        PROC_INBUF->stat_inbufmax = PROC_INBUF->stat_inbufnow;
    }
    return inbuf;
}

/**
 * free an inbuf (not really freed, maintains freelist for reuse)
 *
 * @param INBUF thread context
 * @param inbuf
 */
void free_inbuf(INBUF_t * INBUF, inbufelem_t * inbuf)
{
    PROC_INBUF_t *PROC_INBUF = INBUF->PROC_INBUF;
    assert(inbuf);

    // insert inbuf into inbuf_freelist
    inbuf->u.nextinbuf = PROC_INBUF->free_inbuf_list;
    PROC_INBUF->free_inbuf_list = inbuf;

    PROC_INBUF->stat_inbufnow--;    // stats
}
