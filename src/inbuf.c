#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include "grammar.h"
#include "inbuf.h"
#include "list.h"
#include "context.h"
#include "stats.h"

static inbuf_t *free_inbuf_list;

inbuf_t* new_inbuf(void) {
    inbuf_t *inbuf, *next;
    int i;

    if (! free_inbuf_list) {       // if no inbufs in free_inbuf_list
        
        free_inbuf_list = malloc(INBUFALLOCNUM * sizeof(inbuf_t));
// FIXME - add proper run-time error handling
        assert(free_inbuf_list);
        stat_inbufmalloc++;

        next = free_inbuf_list;    // link the new inbufs into free_inbuf_list
        i = INBUFALLOCNUM;
        while (i--) {
	    inbuf = next++;
	    inbuf->next = next;
        }
	inbuf->next = NULL;        // terminate last inbuf

    }
    inbuf = free_inbuf_list;       // use first inbuf from free_inbuf_list
    free_inbuf_list = inbuf->next; // update list to point to next available

    inbuf->next = NULL;
    inbuf->refs = 0;
    inbuf->end_of_buf = '\0';      // parse() sees this null like an EOF

    stat_inbufnow++;             // stats
    if (stat_inbufnow > stat_inbufmax) {
        stat_inbufmax = stat_inbufnow;
    }
    return inbuf;
}

void free_inbuf(inbuf_t *inbuf) {
    assert(inbuf);
    
    // insert inbuf into inbuf_freelist
    inbuf->next = free_inbuf_list;
    free_inbuf_list = inbuf;

    stat_inbufnow--;             // stats
}

