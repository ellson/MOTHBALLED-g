#include <stdio.h>
#include <assert.h>

#include "grammar.h"
#include "inbuf.h"
#include "list.h"
#include "context.h"
#include "emit.h"

// rewrite list into new list with any EQL elements substituted from oldlist
static void
sameas_r(container_context_t * CC, elem_t * list, elem_t ** nextold,
	 elem_t * newlist)
{
	elem_t *elem, *new, *nextoldelem = NULL;
	elem_t object = { 0 };
	state_t si;

	assert(list->type == (char)LISTELEM);

	elem = list->u.list.first;
	while (elem) {
		si = (state_t) elem->state;
		switch (si) {
		case NODE:
		case EDGE:
			if (CC->act_type == 0) {
				CC->act_type = si;	// record if the ACT has a NODE or EDGE SUBJECT
			} else {
				if (si != CC->act_type) {
					if (si == NODE) {
						emit_error(CC->context, si, "EDGE subject includes");
					} else {
						emit_error(CC->context, si, "NODE subject includes");
					}
				}
			}
			if (*nextold) {
				if (si == (state_t) (*nextold)->state)	// old subject matches NODE or EDGE type
				{
					// doesn't matter if old is shorter
					// ... as long as no forther substitutions are needed
					nextoldelem = (*nextold)->u.list.first;	// in the recursion, iterate over
					// the parts of the NODE or EDGE 
					*nextold = (*nextold)->next;	// at this level, continue over the NODES or EDGES
				} else	// else we have no old, just ignore it
				{
					nextoldelem = NULL;
					*nextold = NULL;
				}
			}
			sameas_r(CC, elem, &nextoldelem, &object);	// recurse, adding result to a sublist
			new = move_list(&object);
            new->state = si;
			append_list(newlist, new);
			break;
		case NODEID:
			new = ref_list(elem);
			append_list(newlist, new);
			if (*nextold) {	// doesn't matter if old is shorter
				// ... as long as no forther substitutions are needed
				*nextold = (*nextold)->next;
			}
			break;
		case EQL:
			if (*nextold) {
				new = ref_list(*nextold);
				append_list(newlist, new);

				*nextold = (*nextold)->next;
			} else {
				emit_error(CC->context, si, "No corresponding object found for same-as substitution");
			}
			break;
		default:
			if (*nextold) {	// doesn't matter if old is shorter
				// ... as long as no forther substitutions are needed
//				new = ref_list(*nextold);
//				append_list(newlist, new);
				nextoldelem = (*nextold)->u.list.first;	// for the recursion
				*nextold = (*nextold)->next;	// at this level, continue over the elems
			}
			sameas_r(CC, elem, &nextoldelem, &object);	// recurse, adding result to a sublist
			new = move_list(&object);
            new->state = si;
			append_list(newlist, new);
			break;
		}
		elem = elem->next;
	}
}

//     rewrite subject into a newsubject
//     compare subject with oldsubject
//     substitue EQL in newsubject from corresponding member of oldsubject (or error if old not available)
//     replace subject and oldsubject with newsubject
void sameas(container_context_t * CC, elem_t * subject)
{
	elem_t *newsubject, *oldsubject, *nextold;
	elem_t subject_rewrite = { 0 };

	newsubject = &subject_rewrite;
	oldsubject = &(CC->subject);
	nextold = oldsubject->u.list.first;

    assert(subject);
    assert((state_t)subject->state == SUBJECT);
    CC->act_type = 0;

	// rewrite subject into newsubject with any EQL elements substituted from oldsubject
	sameas_r(CC, subject, &nextold, newsubject);

	free_list(subject);     // free original subject
                           	//    ( although refs are retained in other lists )
	free_list(oldsubject);	// free the previos oldsubject

    newsubject->state = SUBJECT;  
	*oldsubject = *newsubject;	// save the newsubject as oldsubject
	assert(newsubject->u.list.first);
	newsubject->u.list.first->v.list.refs++;	// and increase its reference count
	*subject = *newsubject; //    to also save as the rewritten current subject
}
