#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "list.h"
#include "grammar.h"

#define LISTALLOCNUM 128

static elem_t *free_elem_list;

// FIXME - need a way to read these
static long int stat_listmemory, stat_elemcount;

static elem_t* new_elem_sub(char type) {
    elem_t *elem, *next;
    int i;

    if (! free_elem_list) {       // if no elems in free_elem_list
        
        free_elem_list = malloc(LISTALLOCNUM * size_elem_t);
// FIXME - add proper run-time error handling
        assert(free_elem_list);
        stat_listmemory += LISTALLOCNUM * size_elem_t;

        next = free_elem_list;  // link the new elems into free_elem_list
        i = LISTALLOCNUM;
        while (i--) {
	    elem = next++;
	    elem->next = next;
        }
	elem->next = NULL; // terminate last elem

    }
    elem = free_elem_list;   // use first elem from free_elem_list
    free_elem_list = elem->next;  // update list to point to next available

    elem->type = type;  // init the new elem
    elem->next = NULL;

    stat_elemcount++;   // stats
    return elem;
}

elem_t* new_frag(char state, unsigned char *frag, int len, void *allocated) {
    elem_t* elem;
    
    elem = new_elem_sub(FRAGELEM);

    elem->u.frag.frag = frag;  // complete frag elem initialization
    elem->len = len;
    elem->state = state;
    elem->u.frag.allocated = allocated;
    return elem;
}

// To allow statically allocated list headers, list2elem
// copies the header into a new elem, and then reinitializes
// the old header.
elem_t *list2elem(elem_t *list, int len) {
    elem_t *elem;

    assert(list->type == LISTELEM);

    elem = new_elem_sub(LISTELEM);

    elem->u.list.first = list->u.list.first;  // complete as elem containing list
    elem->u.list.last = list->u.list.last;
    elem->state = list->state;  // get state from first elem
    elem->len = len;            // caller provides len 
				// -  not used internally by list code

    list->u.list.first = NULL; // reset old header
    list->u.list.last = NULL;
    list->state = 0;
    list->len = 0;

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

// free the list contents, but not the lit header.
void free_list(elem_t *list) {
    elem_t *elem, *next;

    assert(list);
    assert(list->type == LISTELEM );

    // free list of elem, but really just put them backs
    // on the elem_freelist (declared at the top of this file)`
    elem = list->u.list.first;
    while (elem) {
	next = elem->next;
        switch (elem->type) {
        case FRAGELEM :
	    if (elem->u.frag.allocated) { // if the elem containss
				//  an allocated buf, then really free that
	        free(elem->u.frag.frag);
            }
	    break;
        case LISTELEM :
	    free_list(elem);  // recursively free lists of lists
	    break;
	}

        // insert elem at beginning of freelist
        elem->next = free_elem_list;
        free_elem_list = elem;
        stat_elemcount--;

	elem = next;
    }

    // clean up emptied list
    list->u.list.first = NULL;
    list->u.list.last = NULL;
    list->state = 0;
    list->len = 0;
}

void print_frag(FILE *chan, unsigned char len, unsigned char *frag) {
    while (len--) putc(*frag++, chan);
}

int print_string(FILE *chan, unsigned char *len_frag) {
    unsigned char len;

    len = *len_frag++;
    print_frag(chan, len, len_frag);
    return len;
}
        
void print_list(FILE *chan, elem_t *list, int indent, char sep) {
    elem_t *elem;
    char type;
    unsigned char *cp;
    int ind, cnt, len, width;

    assert(list->type == LISTELEM);
    elem = list->u.list.first;
    if (!elem) return;
    type = elem->type;
    switch (type) {
    case FRAGELEM :
        if (sep) putc(sep, chan);
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
        cnt = 0;
        width = 0;
        while (elem) {
            assert(elem->type == type);  // check all the same type
	    if (cnt++) {
		putc ('\n', chan);
		putc (' ', chan);
	        if (indent >= 0) {
		    ind = indent;
	            while (ind--) putc(' ',chan);
                }
	    }
            else {
		putc (' ', chan);
            }
            width = print_string(chan, NAMEP(state_machine+(elem->state)));
            ind = indent + width + 1;;
	    print_list(chan, elem, ind, sep);  // recurse
            elem = elem->next;
	}
	break;
    }
}
