#include <stdlib.h>
#include <assert.h>

#include "inbuf.h"

static inbuf_t *free_inbuf_list;

#define INBUFALLOCNUM 128

// FIXME - need a way to read these
static long int stat_inbufmemory, stat_inbufcount;

inbuf_t* new_inbuf(void) {
    inbuf_t *inbuf, *next;
    int i;

    if (! free_inbuf_list) {    // if no inbufs in free_inbuf_list
        
        free_inbuf_list = malloc(INBUFALLOCNUM * INBUFSIZE);
// FIXME - add proper run-time error handling
        assert(free_inbuf_list);
        stat_inbufmemory += INBUFALLOCNUM * INBUFSIZE;

        next = free_inbuf_list;  // link the new inbufs into free_inbuf_list
        i = INBUFALLOCNUM;
        while (i--) {
	    inbuf = next++;
	    inbuf->next = next;
        }
	inbuf->next = NULL; // terminate last inbuf

    }
    inbuf = free_inbuf_list;   // use first inbuf from free_inbuf_list
    free_inbuf_list = inbuf->next;  // update list to point to next available

    inbuf->next = NULL;
    inbuf->end_of_buf = '\0';  // parse() sees this null like an EOF

    stat_inbufcount++;   // stats
    return inbuf;
}
