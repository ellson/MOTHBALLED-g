#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "list.h"

static elem_t *Freelist;

static elem_t* new_elem_r(elemtype_t type, int state) {
    elem_t *elem, *next;
    int i;

    if (Freelist) {       // if elem in freelist
        elem = Freelist;   // use first
    }
    else {                     // else no elems in freelist
#define LISTALLOCNUM 100
        Freelist = calloc(LISTALLOCNUM, sizeof(elem_t));
        next = &Freelist[0];
        for (i=0; i<LISTALLOCNUM;) {   // ... so add LISTALLOCNUM elems to free list
	    elem = next;
            next = &Freelist[++i];
	    elem->next = next;
        }
	elem->next = NULL;      // terminate last elem
        elem = Freelist;   // add chain of elems to freelist
    }
    Freelist = elem->next;  // update freelist to point to next available
    elem->type = type;
    elem->state = state;
    elem->next = NULL;
    return elem;
}

elem_t* new_list(int state) {
    elem_t* elem;

    elem = new_elem_r(LISTELEM, state);
    elem ->u.list.first = NULL;
    elem ->u.list.last = NULL;
    return elem;
}

elem_t* new_frag(int state, unsigned char *frag, int len, void *allocated) {
    elem_t* elem;
    
    elem = new_elem_r(FRAGELEM, state);
    elem->u.frag.frag = frag;
    elem->len = len;
    elem->u.frag.allocated = allocated;
    return elem;
}

// to allow statically allocated list headers, list2elem copies the header into
// a new elem, and then reinitializes the old header.
elem_t *list2elem(elem_t *list) {
    elem_t *elem;

    assert(list->type == LISTELEM);

    elem = new_list(0);
    elem->u.list.first = list->u.list.first;
    elem->u.list.last = list->u.list.last;
    elem->state = list->state;
    list->u.list.first = NULL;
    list->u.list.last = NULL;
    list->state = 0;
    return elem;
}

void append_list(elem_t *list, elem_t *elem) {

    assert(list->type == LISTELEM);

    if (list->u.list.first) {
	list->u.list.last->next = elem;
    }
    else {
	list->u.list.first = elem;
    }
    list->u.list.last = elem;
}

void prepend_list(elem_t *list, elem_t *elem) {

    assert(list->type == LISTELEM);

    elem->next = list->u.list.first;
    list->u.list.first = elem;
}

void free_list(elem_t *list) {
    elem_t *elem, *next;

    assert(list->type == LISTELEM);

    // free list of elem, but really just put them back on the elem_freelist
    elem = list->u.list.first;
    while (elem) {
	next = elem->next;
        switch (elem->type) {
        case FRAGELEM :
	    if (elem->u.frag.allocated) { // if the elem contains an allocated buf, then really free that
	        free(elem->u.frag.frag);
            }
	    break;
        case LISTELEM :
	    free_list(elem);  // recursively free lists of lists
	    break;
	}

        // insert elem at beginning of freelist
        elem->next = Freelist;
        Freelist = elem;

	elem = next;
    }

    // clean up emptied list
    list->u.list.first = NULL;
    list->u.list.last = NULL;
    list->state = 0;
}
        
static void print_list_r(FILE *chan, elem_t *list, int nest) {
    elem_t *elem;
    elemtype_t type;
    unsigned char *cp;
    int len, cnt;

    assert(list->type == LISTELEM);
    cnt = 0;
    elem = list->u.list.first;
    type = elem->type;
    switch (type) {
    case FRAGELEM :
        while (elem) {
            assert(elem->type == type);  // check all the same type
            cp = elem->u.frag.frag;
            len = elem->len;
	    assert(len > 0);
            while (len--) putc (*cp++, chan);
            elem = elem->next;
        }
        break;
    case LISTELEM :
        while (elem) {
            assert(elem->type == type);  // check all the same type
            if (cnt++) putc (' ', chan);
	    print_list_r(chan, elem, nest++);  // recursively print lists
            elem = elem->next;
	}
	break;
    }
}

void print_list(FILE* chan, elem_t *list) {
    print_list_r(chan, list, 0);
}
