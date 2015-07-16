#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"

static elem_t *elem_freelist;

elem_t* newelem(int type, unsigned char *buf, int len, int allocated) {
    elem_t *elem, *next;
    int i;

    elem = NULL;
    if (elem_freelist) {       // if elem in freelist
        elem = elem_freelist;   // use first
    }
    else {                     // else no elems in freelist
        for (i=0; i<20; i++) {   // ... so add 20 elems 
            next = elem;
	    elem = malloc (sizeof(elem_t));
            elem->next = next;
        }
        elem_freelist = elem;   // add chain of elems to freelist
    }
    elem_freelist = elem->next;  // update freelist to point to next available

    elem->type = type;
    elem->buf = buf;
    elem->len = len;
    elem->allocated = allocated;
    elem->next = NULL;
    return elem;
}

elem_t *joinlist2elem(elemlist_t *list, int type) {
    elem_t *elem, *next, *new;
    int len;
    unsigned char *pos;

    // calc total len ielems to be joined
    len=0;
    elem = list->first;
    while (elem) {
        len += elem->len;
	elem = elem->next;
    }

    elem = list->first;
    if (elem->next == NULL ) {
        // optimize by directly promoting if only one elem,
        new = list->first;
        new->type = type;
    }
    else {
        // prepare new to contain joined list in allocated buffer
        pos = malloc(len+1);
        new = newelem(type, pos, len, 1);
    
        // iterate over list to be joined
        elem = list->first;
        while (elem) {
	    next = elem->next;
    
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
    
	    elem = next;
        }
    
        new->next = NULL;
    }
    list->first = NULL;
    list->last = NULL;
    list->type = 0;
    return new;
}

void appendlist(elemlist_t *list, elem_t *elem) {
    if (list->first) {
	list->last->next = elem;
    }
    else {
	list->first = elem;
    }
    list->last = elem;
}

void freelist(elemlist_t *list) {
    elem_t *elem, *next;

    // free list of elem, but really just put them back on the elem_freelist
    elem = list->first;
    while (elem) {
	next = elem->next;
        if (elem->allocated) { // if the elem containes allocated buf, then really free that
	    free(elem->buf);
	}

        // insert elem at beginning of freelist
        elem->next = elem_freelist;
        elem_freelist = elem;

	elem = next;
    }

    // clean up emptied list
    list->first = NULL;
    list->last = NULL;
    list->type = 0;
}
        
void freefreelist(void) {
    elem_t *elem, *next;

    elem = elem_freelist;
    while (elem) {
	next = elem->next;
	free(elem);
	elem=next;
    }
    elem_freelist = NULL;
}
        
void printj(elemlist_t *list, char *join) {
    elem_t *elem;
    unsigned char *cp;
    int len;

    elem = list->first;
    while (elem) {
        cp = elem->buf;
        len = elem->len;
        while (len--) printf ("%c", *cp++);
        if (elem != list->last) printf (join);
	elem = elem->next;
    }
    printf ("\n");
}
