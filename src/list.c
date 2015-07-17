#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "list.h"

static elem_t *elem_freelist;

static elem_t* newelem_private(elemtype_t type, int props) {
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
    elem->props = props;
    elem->next = NULL;
    return elem;
}

elem_t* newlist(int props) {
    elem_t* elem;

    elem = newelem_private(LST, props);
    elem ->u.lst.first = NULL;
    elem ->u.lst.last = NULL;
    return elem;
}

elem_t* newelem(int props, unsigned char *buf, int len, int allocated) {
    elem_t* elem;
    
    elem = newelem_private(STR, props);
    elem->u.str.buf = buf;
    elem->u.str.len = len;
    elem->u.str.allocated = allocated;
    return elem;
}

// to allow statically allocated list headers, list2elem copies the header into
// a new elem, and then reinitializes the old header.
elem_t *list2elem(elem_t *list) {
    elem_t *elem;

    assert(list->type == LST);

    elem = newlist(0);
    elem->u.lst.first = list->u.lst.first;
    elem->u.lst.last = list->u.lst.last;
    elem->props = list->props;
    list->u.lst.first = NULL;
    list->u.lst.last = NULL;
    list->props = 0;
    return elem;
}

void appendlist(elem_t *list, elem_t *elem) {

    assert(list->type == LST);

    if (list->u.lst.first) {
	list->u.lst.last->next = elem;
    }
    else {
	list->u.lst.first = elem;
    }
    list->u.lst.last = elem;
}

void freelist(elem_t *list) {
    elem_t *elem, *next;

    assert(list->type == LST);

    // free list of elem, but really just put them back on the elem_freelist
    elem = list->u.lst.first;
    while (elem) {
	next = elem->next;
        switch (elem->type) {
        case STR :
	    if (elem->u.str.allocated) { // if the elem contains an allocated buf, then really free that
	        free(elem->u.str.buf);
            }
	    break;
        case LST :
	    freelist(elem);  // recursively free lists of lists
	    break;
	}

        // insert elem at beginning of freelist
        elem->next = elem_freelist;
        elem_freelist = elem;

	elem = next;
    }

    // clean up emptied list
    list->u.lst.first = NULL;
    list->u.lst.last = NULL;
    list->props = 0;
}
        
static void printj_private(elem_t *list, int indent) {
    elem_t *elem;
    unsigned char *cp;
    int len, cnt;

    assert(list->type == LST);
    cnt = 0;
    elem = list->u.lst.first;
    switch (elem->type) {
    case STR :
        while (elem) {
            assert(elem->type == STR);
            cp = elem->u.str.buf;
            len = elem->u.str.len;
            if (len) {
                while (indent--) putc (' ', stdout);
                while (len--) putc (*cp++, stdout);
                cnt++;
            }
            elem = elem->next;
        }
        if (cnt) {
            putc ('\n', stdout);
        }
        break;
    case LST :
        while (elem) {
            assert(elem->type == LST);
	    printj_private(elem, indent+2);  // recursively print lists
            elem = elem->next;
	}
	break;
    }
}

void printj(elem_t *list) {
    printj_private(list, 0);
}
