#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "grammar.h"
#include "inbuf.h"

#define INBUFALLOCNUM 128
#define LISTALLOCNUM 128

static inbuf_t *free_inbuf_list;
static elem_t *free_elem_list;

// FIXME - need a way to read these
static long int stat_inbufmemory, stat_inbufcount;
static long int stat_listmemory, stat_elemcount;


inbuf_t* new_inbuf(void) {
    inbuf_t *inbuf, *next;
    int i;

    if (! free_inbuf_list) {       // if no inbufs in free_inbuf_list
        
        free_inbuf_list = malloc(INBUFALLOCNUM * sizeof(inbuf_t));
// FIXME - add proper run-time error handling
        assert(free_inbuf_list);
        stat_inbufmemory += INBUFALLOCNUM * sizeof(inbuf_t);

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

    stat_inbufcount++;             // stats
    return inbuf;
}

static void free_inbuf(inbuf_t *inbuf) {
    assert(inbuf);
    
    // insert inbuf into inbuf_freelist
    inbuf->next = free_inbuf_list;
    free_inbuf_list = inbuf;

    stat_inbufcount--;             // stats
}

unsigned char * more_in(context_t *C) {
    if (C->size != INBUFSIZE) {    // if previous inbuf was short,
        if (C->size != -1) {       //     unless its a new new stream 
	    return NULL;           //         EOF
        }
    }
    C->inbuf = new_inbuf();        // grab a buffer
    assert(C->inbuf);

    C->size = fread(C->inbuf->buf, 1, INBUFSIZE, C->file); // slurp in data from file stream

    return C->inbuf->buf;
}

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

elem_t* new_frag(char state, unsigned char *frag, int len, inbuf_t *inbuf) {
    elem_t* elem;
    
    elem = new_elem_sub(FRAGELEM);

    elem->u.frag.frag = frag;  // complete frag elem initialization
    elem->len = len;
    elem->state = state;
    elem->u.frag.inbuf = inbuf;

    assert (inbuf);
    assert (inbuf->refs >= 0);

    inbuf->refs++;             // increment reference count in inbuf.
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
	    assert(elem->u.frag.inbuf->refs > 0);
	    if (--(elem->u.frag.inbuf->refs)) {
		free_inbuf(elem->u.frag.inbuf);
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
