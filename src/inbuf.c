#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "grammar.h"
#include "inbuf.h"

#define INBUFALLOCNUM 128
#define LISTALLOCNUM 128

static inbuf_t *free_inbuf_list;
static elem_t *free_elem_list;

static long int stat_inbufmalloc, stat_inbufmax, stat_inbufcount;
static long int stat_elemmalloc, stat_elemmax, stat_elemcount;

void print_stats(FILE *chan) {
    fprintf(chan,"\n");
    fprintf(chan,"stats [\n");
    fprintf(chan," inbufmalloc=%ld\n", stat_inbufmalloc);
    fprintf(chan,"   inbufsize=%ld\n", sizeof(inbuf_t));
    fprintf(chan,"    inbufmax=%ld\n", stat_inbufmax);
    fprintf(chan,"  inbufcount=%ld\n", stat_inbufcount);
    fprintf(chan,"  elemmalloc=%ld\n", stat_elemmalloc);
    fprintf(chan,"    elemsize=%ld\n", size_elem_t);
    fprintf(chan,"     elemmax=%ld\n", stat_elemmax);
    fprintf(chan,"   elemcount=%ld\n", stat_elemcount);
    fprintf(chan,"]\n");
}

static inbuf_t* new_inbuf(void) {
    inbuf_t *inbuf, *next;
    int i;

    if (! free_inbuf_list) {       // if no inbufs in free_inbuf_list
        
        free_inbuf_list = malloc(INBUFALLOCNUM * sizeof(inbuf_t));
// FIXME - add proper run-time error handling
        assert(free_inbuf_list);
        stat_inbufmalloc += INBUFALLOCNUM * sizeof(inbuf_t);

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
    if (stat_inbufcount > stat_inbufmax) stat_inbufmax = stat_inbufcount;
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
        stat_elemmalloc += LISTALLOCNUM * size_elem_t;

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
    if (stat_elemcount > stat_elemmax) {
	stat_elemmax = stat_elemcount;
    }
    return elem;
}

elem_t* new_frag(char state, unsigned char *frag, int len, inbuf_t *inbuf) {
    elem_t* elem;
    
    elem = new_elem_sub(FRAGELEM);

    assert (inbuf);
    assert (inbuf->refs >= 0);
    assert (frag);
    assert (len > 0);

				// complete frag elem initialization
    elem->u.frag.inbuf = inbuf; // record inbuf for ref counting
    elem->u.frag.frag = frag;   // pointer to begging of frag
    elem->v.frag.len = len;     // length of frag
    elem->state = state;        // state_machine state that created this frag

    inbuf->refs++;              // increment reference count in inbuf.
    return elem;
}

// clone_list -  clone a list header to a new elem
//  -- ref count in first elem is not updated
static elem_t *clone_list(elem_t *list) {
    elem_t *elem;

    assert(list->type == LISTELEM);

    elem = new_elem_sub(LISTELEM);

    elem->u.list.first = list->u.list.first;  // copy details
    elem->u.list.last = list->u.list.last;
    elem->v.list.refs = 0;
    return elem;
}

// move_list - move a list to a new elem
//     implemented using clone_list
//         clone_list didn't increase ref count in first elem,
//         so no need to deref.
//     clean up the old list header so it no longer references the list elems.
elem_t *move_list(elem_t *list) {
    elem_t *elem;

    elem = clone_list(list);

    list->u.list.first = NULL; // reset old header
    list->u.list.last = NULL;

    return elem;
}

// ref_list - reference a list from a new elem
//     implement as a clone_list with a ref count adjustment
//     if there is a first elem and if it is a LISTELEM, then
//           increment its ref count.
elem_t *ref_list(elem_t *list) {
    elem_t *elem;

    elem = clone_list(list);

    if (list->u.list.first
     && list->u.list.first->type == LISTELEM) {
        list->u.list.first->v.list.refs++;   // increment ref count
    }
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
    if (elem->type == LISTELEM) {
        elem->v.list.refs++;   // increment ref count in appended elem
    }
}

// free the list contents, but not the list header.
void free_list(elem_t *list) {
    elem_t *elem, *next;

    assert(list);
    assert(list->type == LISTELEM );

    // free list of elem, but really just put them back
    // on the elem_freelist (declared at the top of this file)`
    elem = list->u.list.first;
    while (elem) {
	next = elem->next;
        switch (elem->type) {
        case FRAGELEM :
	    assert(elem->u.frag.inbuf->refs > 0);
	    if (--(elem->u.frag.inbuf->refs) == 0) {
		free_inbuf(elem->u.frag.inbuf);
	    }
	    break;
        case LISTELEM :
	    assert(elem->v.list.refs > 0);
            if(--(elem->v.list.refs) == 0) {
	        free_list(elem);  // recursively free lists that have no references
            }
	    break;
	}

        // insert elem at beginning of freelist
        elem->next = free_elem_list;
        free_elem_list = elem;

        stat_elemcount--;         // maintain stats

	elem = next;
    }

    // clean up emptied list
    list->u.list.first = NULL;
    list->u.list.last = NULL;
    // Note: ref count of header is not modified
}

void print_frag(FILE *chan, unsigned char len, unsigned char *frag) {
    while (len--) putc(*frag++, chan);
}

int print_len_frag(FILE *chan, unsigned char *len_frag) {
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
            len = elem->v.frag.len;
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
            width = print_len_frag(chan, NAMEP(state_machine+(elem->state)));
            ind = indent + width + 1;;
	    print_list(chan, elem, ind, sep);  // recurse
            elem = elem->next;
	}
	break;
    }
}
