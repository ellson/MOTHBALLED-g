#include "libje_private.h"

static elem_t *new_elem_sub(context_t * C, elemtype_t type)
{
	elem_t *elem, *next;
	int i;

	if (!C->free_elem_list) {	// if no elems in free_elem_list

		C->free_elem_list = malloc(LISTALLOCNUM * size_elem_t);
        if (!C->free_elem_list) {
            perror("Error - malloc(): ");
            exit(EXIT_FAILURE);
        }
		C->stat_elemmalloc++;

		next = C->free_elem_list;	// link the new elems into free_elem_list
		i = LISTALLOCNUM;
		while (i--) {
			elem = next++;
			elem->next = next;
		}
		elem->next = NULL;	// terminate last elem

	}
	elem = C->free_elem_list;	// use first elem from free_elem_list
	C->free_elem_list = elem->next;	// update list to point to next available

	elem->type = (char)type;	// init the new elem
	elem->next = NULL;

	C->stat_elemnow++;		// stats
	if (C->stat_elemnow > C->stat_elemmax) {
		C->stat_elemmax = C->stat_elemnow;
	}
	return elem;
}

elem_t *new_frag(context_t * C, char state, int len, unsigned char *frag)
{
	elem_t *elem;

	elem = new_elem_sub(C, FRAGELEM);

	assert(C->inbuf);
	assert(C->inbuf->refs >= 0);
	assert(frag);
	assert(len > 0);
	// complete frag elem initialization
	elem->u.frag.inbuf = C->inbuf;	// record inbuf for ref counting
	elem->u.frag.frag = frag;	// pointer to begging of frag
	elem->v.frag.len = len;	// length of frag
	elem->state = state;	// state_machine state that created this frag

	C->inbuf->refs++;		// increment reference count in inbuf.
	return elem;
}

elem_t *new_hash(context_t * C, unsigned long hash)
{
	elem_t *elem;

	elem = new_elem_sub(C, HASHELEM);

	// complete frag elem initialization
	elem->u.hash.hash = hash;	// the hash value
	elem->u.hash.out = NULL;    // open later
	return elem;
}

// clone_list -  clone a list header to a new elem
//  -- ref count in first elem is not updated
//     so this function is only for use by move_list() or ref_list()
static elem_t *clone_list(context_t * C, elem_t * list)
{
	elem_t *elem;

	assert(list->type == (char)LISTELEM);

	elem = new_elem_sub(C, LISTELEM);

	elem->u.list.first = list->u.list.first;	// copy details
	elem->u.list.last = list->u.list.last;
	elem->v.list.refs = 0;
	elem->state = list->state;
	return elem;
}

// move_list - move a list to a new elem
//     implemented using clone_list
//         clone_list didn't increase ref count in first elem,
//         so no need to deref.
//     clean up the old list header so it no longer references the list elems.
elem_t *move_list(context_t * C, elem_t * list)
{
	elem_t *elem;

	elem = clone_list(C, list);

	list->u.list.first = NULL;	// reset old header
	list->u.list.last = NULL;
    list->state = 0;

	return elem;
}

// ref_list - reference a list from a new elem
//     implement as a clone_list with a ref count adjustment
//     if there is a first elem and if it is a LISTELEM, then
//           increment the first elems ref count.  (NB, not this new elem)
elem_t *ref_list(context_t * C, elem_t * list)
{
	elem_t *elem;

	elem = clone_list(C, list);

	if (list->u.list.first && list->u.list.first->type == LISTELEM) {
		list->u.list.first->v.list.refs++;	// increment ref count
	}
	return elem;
}

// append elem to the end of the list so that the elem becomes
//     the new u.list.last
void append_list(elem_t * list, elem_t * elem)
{
	assert(list->type == (char)LISTELEM);

	if (list->u.list.first) {
		list->u.list.last->next = elem;
	} else {
		list->u.list.first = elem;
	}
	list->u.list.last = elem;
	if (elem->type == (char)LISTELEM) {
		elem->v.list.refs++;	// increment ref count in appended elem
		assert(elem->v.list.refs > 0);
	}
}

// free the list contents, but not the list header.
// ( this function can be used on statically or stack allocated list headers )
void free_list(context_t * C, elem_t * list)
{
	elem_t *elem, *next;

	assert(list);
	assert(list->type == (char)LISTELEM);

	// free list of elem, but really just put them back
	// on the elem_freelist (declared at the top of this file)`
	elem = list->u.list.first;
	while (elem) {
		next = elem->next;
		switch ((elemtype_t) (elem->type)) {
		case FRAGELEM:
			assert(elem->u.frag.inbuf->refs > 0);
			if (--(elem->u.frag.inbuf->refs) == 0) {
				free_inbuf(C, elem->u.frag.inbuf);
			}
			break;
		case LISTELEM:
			assert(elem->v.list.refs > 0);
			if (--(elem->v.list.refs) > 0) {
				goto done;	// stop at any point with additional refs
			}
			free_list(C, elem);	// recursively free lists that have no references
			break;
		case HASHELEM:
            assert(0);  // should not be here
            break;
		}

		// insert elem at beginning of freelist
		elem->next = C->free_elem_list;
		C->free_elem_list = elem;

		C->stat_elemnow--;	// maintain stats

		elem = next;
	}

 done:
	// clean up emptied list
	list->u.list.first = NULL;
	list->u.list.last = NULL;
	// Note: ref count of header is not modified
}

static void print_one_frag(FILE * chan, unsigned char len, unsigned char *frag)
{
	while (len--) {
		putc(*frag++, chan);
    }
}

int print_len_frag(FILE * chan, unsigned char *len_frag)
{
	unsigned char len;

	len = *len_frag++;
	print_one_frag(chan, len, len_frag);
	return len;
}

void print_frags(FILE * chan, state_t liststate, elem_t * elem, char *sep)
{
    unsigned char *frag;
    int len;

    if (*sep) {
        putc(*sep, chan);
    }
    if (liststate == DQT) {
        putc('"', chan);
    }
    while (elem) {
        frag = elem->u.frag.frag;
        len = elem->v.frag.len;
        assert(len > 0);
        if ((state_t) elem->state == BSL) {
            putc('\\', chan);
        }
        if ((state_t) elem->state == AST) {
            if (liststate == DQT) {
                putc('"', chan);
                putc('*', chan);
                putc('"', chan);
            } else {
                putc('*', chan);
            }
        }
        else {
            print_one_frag(chan, len, frag);
        }
        elem = elem->next;
    }
    if (liststate == DQT) {
        putc('"', chan);
    }
    *sep = ' ';
}

void print_list(FILE * chan, elem_t * list, int indent, char *sep)
{
	elem_t *elem;
	elemtype_t type;
	int ind, cnt, width;

	assert(list->type == (char)LISTELEM);
	elem = list->u.list.first;
	if (!elem)
		return;
	type = (elemtype_t) (elem->type);
	switch (type) {
	case FRAGELEM:
        print_frags(chan, list->state, elem, sep);
		break;
	case LISTELEM:
		cnt = 0;
		width = 0;
		while (elem) {
			assert(elem->type == (char)type);	// check all the same type
			if (cnt++) {
				putc('\n', chan);
				putc(' ', chan);
				if (indent >= 0) {
					ind = indent;
					while (ind--)
						putc(' ', chan);
				}
			} else {
				putc(' ', chan);
			}
			width = print_len_frag(chan, NAMEP(elem->state));
			ind = indent + width + 1;;
			print_list(chan, elem, ind, sep);	// recurse
			elem = elem->next;
		}
		break;
	case HASHELEM:
        assert(0);  // should not be here
        break;
	}
}
