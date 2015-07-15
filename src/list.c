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
    int cnt, len;
    unsigned char *pos;

    // count elems, and calc total len ielems to be joined
    cnt=0;
    len=0;
    elem = list->first;
    while (elem) {
        cnt++;
        len += elem->len;
	elem = elem->next;
    }

#if 0
    switch (cnt) {
    case 0 :
        return NULL;
	break;
    case 1 :
        // optimize by directly promoting of only one elem,
        //  -- unfortunately may be non-'\0'-terminated string
        new = list->first;
        new->type = type;
        break;
    default :
#endif
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
    
        // null terminate for safety and convenience
        *pos = '\0';
    
        new->next = NULL;
#if 0
    }
#endif
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
	list->last = elem;
    }
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
        
