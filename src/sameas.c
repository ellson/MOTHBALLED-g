#include <stdio.h>
#include <assert.h>

#include "grammar.h"
#include "inbuf.h"
#include "list.h"
#include "context.h"
#include "emit.h"

static void print_subject_r(FILE *chan, elem_t *list) {
    elem_t *elem;
    elemtype_t type;
    unsigned char *cp;
    int len;

    assert(list);
    elem = list->u.list.first;
    assert(elem);

    len = 0;
    type = (elemtype_t)elem->type;
    switch (type) {
    case FRAGELEM:
        if (list->state == DQT) {
            putc ('"', chan);
        }
        while (elem) {
            cp = elem->u.frag.frag;
            len = elem->v.frag.len;
            assert(len > 0);
            if (elem->state == BSL) {
                putc ('\\', chan);
            }
            while (len--) putc (*cp++, chan);
            elem = elem->next;
        }
        if (list->state == DQT) {
            putc ('"', chan);
        }
        break;
    case LISTELEM:
        if ((state_t)list->state == ENDPOINTSET) {
            putc('(', chan);
        }
        while (elem) {
            if (len++) {
		putc(' ', chan);
            }
            print_subject_r(chan, elem);  // recurse
            elem = elem->next;
        }
        if ((state_t)list->state == ENDPOINTSET) {
            putc(')', chan);
        }
        break;
    }    
}

void print_subject(context_t *C, elem_t *list) {
    FILE *chan;
    elem_t *elem;

    assert(C);
    chan = C->out;

    assert(list);

    elem = list->u.list.first;
    assert(elem);

    if (list->u.list.first->next) {
	putc('(', chan);
    }
   
    while (elem) {
	if ((state_t)list->state == EDGE) {
	    putc('<', chan);
	}
 
        print_subject_r(chan, elem);

	if ((state_t)list->state == EDGE) {
	    putc('>', chan);
	}
        else {
	    putc(' ', chan);
	}
	elem = elem->next;
    }
    
    if (list->u.list.first->next) {
	putc(')', chan);
    }
}

// flatten list into new list with any EQL elements substituted from oldlist
static void sameas_r(container_context_t *CC, elem_t *list, elem_t **nextold, elem_t *newlist) {
    elem_t *elem, *new, *nextoldelem = NULL;
    elem_t object = {0};
    state_t si;

    assert (list->type == (char)LISTELEM);

    elem = list->u.list.first;
    while(elem) {
        si = (state_t)elem->state;
	switch (si) {
        case NODE:
        case EDGE:
            if (newlist->state == 0) {
		newlist->state = si;
	    }
	    else {
		if (si != (state_t)newlist->state) {
                    if (si == NODE) {
			emit_error(CC->context, si, "EDGE subject includes");
		    }
		    else {
			emit_error(CC->context, si, "NODE subject includes");
		    }
		}
	    }
	    if (*nextold) {                   // doesn't matter if old is shorter
					      // ... as long as no forther substitutions are needed
	        nextoldelem = (*nextold)->u.list.first;  // in the recursion, iterate over the members of the NODE or EDGE SUBJECT
	        *nextold = (*nextold)->next;  // at this level, continue over the NODES or EDGES
            }
	    sameas_r(CC, elem, &nextoldelem, &object);   // recurse, adding result to a sublist
            new = move_list(si,&object);
            append_list(newlist, new);
            break;
        case NODEID:
	    new = ref_list(elem->state, elem);
	    append_list(newlist, new); 
	    if (*nextold) {                   // doesn't matter if old is shorter
					      // ... as long as no forther substitutions are needed
	        *nextold = (*nextold)->next;
            }
	    break;
	case EQL:
	    if (*nextold) {
	        new = ref_list((*nextold)->state, *nextold);
	        append_list(newlist, new); 

	        *nextold = (*nextold)->next;
            }
	    else {
		emit_error(CC->context, si, "No corresponding object found for same-as substitution");
            }
	    break;
        case ENDPOINT:
        case ENDPOINTSET:
	    if (*nextold) {                   // doesn't matter if old is shorter
					      // ... as long as no forther substitutions are needed
	        nextoldelem = (*nextold)->u.list.first;  // in the recursion, iterate over the members of the NODE or EDGE SUBJECT
	        *nextold = (*nextold)->next;  // at this level, continue over the NODES or EDGES
            }
	    sameas_r(CC, elem, &nextoldelem, &object);   // recurse, adding result to a sublist
            new = move_list(si,&object);
            append_list(newlist, new);
	    break;
        default:
	    sameas_r(CC, elem, nextold, newlist);   // recurse
	    break;
	}
	elem = elem->next;
    }
}

//     flatten SUBJECT into a list of ENDPOINT or NODEID
//     compare new flattened list with old (prev subject, already flattened)
//     substitue EQL in new from corresponding member of old (or error if old not available)
//     free prev_subject
//     save new (fully flattened and EQL substituted) list as prev_subject
success_t sameas(container_context_t *CC, elem_t *list) {
    success_t rc;
    elem_t *newlist, *oldlist, *nextold;
    elem_t flattened_subject = {0};

    newlist = &flattened_subject;
    oldlist = &(CC->prev_subject);
    nextold = oldlist->u.list.first;

    // flatten list into new list with any EQL elements substituted from oldlist
    sameas_r(CC, list, &nextold, newlist);

    free_list(list);        // free original tree ( although refs are retained in other lists )
    free_list(oldlist);     // update prev_subject for same_end substitution
    *oldlist = *newlist;    // transfers all refs ... newlist will be out of scope shortly

//putc ('\n', stdout);
//print_list(stdout, newlist, 0, ' ');
//putc ('\n', stdout);

    emit_subject(CC->context, newlist);

    rc = SUCCESS;
    return rc;
}
