#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "list.h"

static elem_t *elem_freelist;

static elem_t* newelem_private(elemtype_t type, int state) {
    elem_t *elem, *next;
    int i;

    if (elem_freelist) {       // if elem in freelist
        elem = elem_freelist;   // use first
    }
    else {                     // else no elems in freelist
#define LISTALLOCNUM 100
        elem_freelist = malloc (LISTALLOCNUM * sizeof(elem_t));
        next = elem_freelist;
        for (i=0; i<LISTALLOCNUM; i++) {   // ... so add LISTALLOCNUM elemes to free list
	    elem = next;
            next = elem + sizeof(elem_t);
	    elem->next = next;
        }
	elem->next = NULL;      // terminate last elem
        elem = elem_freelist;   // add chain of elems to freelist
    }
    elem_freelist = elem->next;  // update freelist to point to next available
    elem->type = type;
    elem->state = state;
    elem->next = NULL;
    return elem;
}

elem_t* newlist(int state) {
    elem_t* elem;

    elem = newelem_private(LIST, state);
    elem ->u.list.first = NULL;
    elem ->u.list.last = NULL;
    return elem;
}

elem_t* newfrag(int state, unsigned char *frag, int len, int allocated) {
    elem_t* elem;
    
    elem = newelem_private(FRAG, state);
    elem->u.frag.frag = frag;
    elem->u.frag.len = len;
    elem->u.frag.allocated = allocated;
    return elem;
}

// to allow statically allocated list headers, list2elem copies the header into
// a new elem, and then reinitializes the old header.
elem_t *list2elem(elem_t *list) {
    elem_t *elem;

    assert(list->type == LIST);

    elem = newlist(0);
    elem->u.list.first = list->u.list.first;
    elem->u.list.last = list->u.list.last;
    elem->state = list->state;
    list->u.list.first = NULL;
    list->u.list.last = NULL;
    list->state = 0;
    return elem;
}

void appendlist(elem_t *list, elem_t *elem) {

    assert(list->type == LIST);

    if (list->u.list.first) {
	list->u.list.last->next = elem;
    }
    else {
	list->u.list.first = elem;
    }
    list->u.list.last = elem;
}

void prependlist(elem_t *list, elem_t *elem) {

    assert(list->type == LIST);

    elem->next = list->u.list.first;
    list->u.list.first = elem;
}

void freelist(elem_t *list) {
    elem_t *elem, *next;

    assert(list->type == LIST);

    // free list of elem, but really just put them back on the elem_freelist
    elem = list->u.list.first;
    while (elem) {
	next = elem->next;
        switch (elem->type) {
        case FRAG :
	    if (elem->u.frag.allocated) { // if the elem contains an allocated buf, then really free that
	        free(elem->u.frag.frag);
            }
	    break;
        case LIST :
	    freelist(elem);  // recursively free lists of lists
	    break;
	}

        // insert elem at beginning of freelist
        elem->next = elem_freelist;
        elem_freelist = elem;

	elem = next;
    }

    // clean up emptied list
    list->u.list.first = NULL;
    list->u.list.last = NULL;
    list->state = 0;
}
        
static void printj_private(elem_t *list, int indent) {
    elem_t *elem;
    elemtype_t type;
    unsigned char *cp;
    int len, cnt;

    assert(list->type == LIST);
    cnt = 0;
    elem = list->u.list.first;
    type = elem->type;
    switch (type) {
    case FRAG :
        while (elem) {
            assert(elem->type == type);  // check all the same type
            cp = elem->u.frag.frag;
            len = elem->u.frag.len;
            if (len) {
        	if (! cnt++) while (indent--) putc (' ', stdout);
                while (len--) putc (*cp++, stdout);
            }
            elem = elem->next;
        }
        if (cnt) putc ('\n', stdout);
        break;
    case LIST :
        while (elem) {
            assert(elem->type == type);  // check all the same type
	    printj_private(elem, indent+2);  // recursively print lists
            elem = elem->next;
	}
	break;
    }
}

void printj(elem_t *list) {
    printj_private(list, 0);
}
