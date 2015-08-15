#include <stdio.h>
#include <assert.h>

#include "grammar.h"
#include "inbuf.h"
#include "list.h"
#include "context.h"
#include "emit.h"


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
	    if (*nextold) {                 // doesn't matter if old is shorter
fprintf (stdout, "\n nextold:\n");
print_list(stdout, *nextold, 0, ' ');
putc ('\n', stdout);
					    // ... as long as no forther substitutions are needed
	        nextoldelem = (*nextold)->u.list.first;
            }
	    sameas_r(CC, elem, &nextoldelem, &object);   // recurse, adding result to a sublist
            new = move_list(si,&object);
            append_list(newlist, new);
            break;
        case NODEID:
        case ENDPOINT:
	    new = ref_list(elem->state, elem);
	    append_list(newlist, new); 
	    if (*nextold) {                 // doesn't matter if old is shorter
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
        case ENDPOINTSET:
	   // FIXME - enpoint sets need expanding
	   sameas_r(CC, elem, nextold, newlist);   // recurse
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

fprintf (stdout, "\n oldlist:\n");
print_list(stdout, oldlist, 0, ' ');
putc ('\n', stdout);

    free_list(list);        // free original tree ( although refs are retained in other lists )
    free_list(oldlist);     // update prev_subject for same_end substitution
    *oldlist = *newlist;    // transfers all refs ... newlist will be out of scope shortly

fprintf (stdout, "\n newlist:\n");
print_list(stdout, newlist, 0, ' ');
putc ('\n', stdout);

    rc = SUCCESS;
    return rc;
}
