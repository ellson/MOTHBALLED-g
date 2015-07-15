#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"

static elem_t *elem_freelist;

elem_t* newelem(int type, unsigned char *buf, int len, int allocated, elem_t *next) {
    elem_t *elem, *this;
    int i;

    elem = NULL;
    if (elem_freelist) {       // if elem in freelist
        elem = elem_freelist;   // use first
    }
    else {                     // else no elems in freelist
        for (i=0; i<20; i++) {   // ... so add 20 elems 
            this = elem;
	    elem = malloc (sizeof(elem_t));
            elem->next = this;
        }
        elem_freelist = elem;   // add chain of elems to freelist
     }
     elem_freelist = elem->next;  // update freelist to point to next available

     elem->type = type;
     elem->buf = buf;
     elem->len = len;
     elem->allocated = allocated;
     elem->next = next;
     return elem;
}

elem_t *joinlist2elem(elem_t *list, int type, elem_t *next) {
    elem_t *elem, *this, *new;
    int len=0;
    unsigned char *pos;

    // calc len of joined list
    elem = list;
    while (elem) {
        len += elem->len;
	elem = elem->next;
    }

    // prepare new to contain joined list in allocated buffer
    pos = malloc(len+1);
    new = newelem(type, pos, len, 1, NULL);

    // iterate over list to be joined
    elem = list;
    while (elem) {
	this = elem->next;

        // copy fragment into new
        len = elem->len;
	strncpy((char*)(pos), (char*)(elem->buf), len);
	pos += len;

        // clean up old and return to elem_freelist
        if (elem->allocated) {
            free(elem->buf);
        }
        elem->next = elem_freelist;
        elem_freelist = elem;

	elem = this;
    }

    // null terminate for safety and convenience
    *pos = '\0';

    new->type = type;
    new->next = next;
    return new;
}

void freelist(elem_t *list) {
    elem_t *elem, *this;

    // free a list of elem, but really just put them back on the elem_freelist
    elem = list;
    while (elem) {
	this = elem->next;
        if (elem->allocated) {
	    free(elem->buf);
	}
        elem->next = elem_freelist;
        elem_freelist = elem;
	elem=this;
    }
    elem_freelist = NULL;
}
        
void freefreelist(void) {
    elem_t *elem, *this;

    elem = elem_freelist;
    while (elem) {
	this = elem->next;
	free(elem);
	elem=this;
    }
    elem_freelist = NULL;
}
        
