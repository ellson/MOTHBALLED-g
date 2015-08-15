#include <stdio.h>
#include <assert.h>

#include "grammar.h"
#include "inbuf.h"
#include "list.h"
#include "context.h"
#include "emit.h"


static void sameas_r(container_context_t *CC, elem_t *list, elem_t *oldlist, elem_t *newlist) {
    elem_t *elem, *old, *new;
    state_t si;

    assert (list->type == (char)LISTELEM);

    elem = list->u.list.first;
    old = oldlist->u.list.first;
    while(elem) {
        si = (state_t)elem->state;
	switch (si) {
        case NODEID:
        case ENDPOINT:
        case ENDPOINTSET:
	    new = ref_list(elem->state, elem);
	    append_list(newlist, new); 
	    elem = elem->next;
	    if (old) {
	        old = old->next;
            }
	    continue;
	case EQL:
	    if (old) {
	        new = ref_list(old->state, old);
	        append_list(newlist, new); 
	        elem = elem->next;
	        old = old->next;
            }
	    else {
		emit_error(CC->context, si, "No corresponding object found for same-as substitution");
            }
	    continue;
        default:
	    break;
	}
	sameas_r(CC, elem, oldlist, newlist);   // recurse
	elem = elem->next;
	if (old) {
	    old = old->next;
        }
    }
}

//     flatten SUBJECT into a list of LEGS (or ENDPOINT or NODEID ?)
//     compare flattened list with prev_subject (already flattened)
//     substitue sameends (or error if no leg exists at index of EQL)
//     free prev_subject
//     save new (fully substitued and flattened) subject as prev_subject
success_t sameas(container_context_t *CC, elem_t *list) {
    success_t rc;
    elem_t *elem, *newlist, *oldlist;;
    elem_t subject = {0};

    newlist = &subject;
    oldlist = &(CC->prev_subject);

    sameas_r(CC, list, oldlist, newlist);

    free_list(list);
    free_list(oldlist);   // update prev_subject for same_end substitution
    *oldlist = *newlist;    // transfers all refs ... newlist will be out of scope shortly

putc ('\n', stdout);
print_list(stdout, newlist, 0, ' ');
putc ('\n', stdout);

    rc = SUCCESS;
    return rc;
}



#if 0
            if (bi == EQL) {
                if (! sameend_elem) {
                    emit_error(C, si, "No prior LEG found for sameend substitution in");
                }
//              elem = ref_list(si, elem);

                elem = ref_list(si, sameend_elem);
// FIXME can be multiple ENDPOINTS in a LEG, need a while here
//                append_list(&branch, sameend_elem->u.list.first);
            }
            if (sameend_elem) {
                sameend_elem = sameend_elem -> next;
            }
#endif
