#include <stdio.h>
#include <assert.h>

#include "grammar.h"
#include "inbuf.h"
#include "list.h"
#include "context.h"
#include "emit.h"


// flatten list into new list with any EQL elements substituted from oldlist
static void sameas_r(container_context_t *CC, elem_t *list, elem_t **nextold, elem_t *newlist) {
    elem_t *elem, *new;
    state_t si;

    assert (list->type == (char)LISTELEM);

    elem = list->u.list.first;
    while(elem) {
        si = (state_t)elem->state;
	switch (si) {
        case NODEID:
        case ENDPOINT:
        case ENDPOINTSET:
	    new = ref_list(elem->state, elem);
	    append_list(newlist, new); 
	    elem = elem->next;
	    if (*nextold) {                 // doesn't matter if old is shorter
					    // ... as long as no forther substitutions are needed
	        *nextold = (*nextold)->next;
            }
	    continue;
	case EQL:
	    if (*nextold) {
	        new = ref_list((*nextold)->state, *nextold);
	        append_list(newlist, new); 
	        elem = elem->next;
	        *nextold = (*nextold)->next;
            }
	    else {
		emit_error(CC->context, si, "No corresponding object found for same-as substitution");
            }
	    continue;
        default:
	    break;
	}
	sameas_r(CC, elem, nextold, newlist);   // recurse
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

putc ('\n', stdout);
print_list(stdout, newlist, 0, ' ');
putc ('\n', stdout);

    rc = SUCCESS;
    return rc;
}
