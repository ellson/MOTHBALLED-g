/* vim:set shiftwidth=4 ts=8 expandtab: */

#include "libje_private.h"
static void je_sameas_r(container_context_t * CC, elem_t * list, elem_t ** nextold, elem_t * newlist);

//     rewrite subject into a newsubject
//     compare subject with oldsubject
//     substitue EQL in newsubject from corresponding member of oldsubject (or error if old not available)
//     replace subject and oldsubject with newsubject
void je_sameas(container_context_t * CC, elem_t * subject)
{
    elem_t *newsubject, *oldsubject, *nextold;
    elem_t subject_rewrite = { 0 };
    context_t * C = CC->context;

    newsubject = &subject_rewrite;
    oldsubject = &(CC->subject);
    nextold = oldsubject->u.list.first;

    assert(subject);
    assert((state_t)subject->state == SUBJECT);
    CC->subject_type = 0;

    // rewrite subject into newsubject with any EQL elements substituted from oldsubject
    je_sameas_r(CC, subject, &nextold, newsubject);

    free_list(C, subject);     // free original subject ( although refs are retained in other lists )
    free_list(C, oldsubject);    // free the previous oldsubject

    newsubject->state = SUBJECT;  
    *oldsubject = *newsubject;    // save the newsubject as oldsubject
    assert(newsubject->u.list.first);
    newsubject->u.list.first->v.list.refs++;    // and increase its reference count
    *subject = *newsubject; //    to also save as the rewritten current subject
}

// rewrite list into new list with any EQL elements substituted from oldlist
static void je_sameas_r(container_context_t * CC, elem_t * list, elem_t ** nextold, elem_t * newlist)
{
    elem_t *elem, *new, *nextoldelem = NULL;
    elem_t object = { 0 };
    state_t si;
    context_t * C = CC->context;

    assert(list->type == (char)LISTELEM);

    elem = list->u.list.first;
    while (elem) {
        si = (state_t) elem->state;
        switch (si) {
        case NODE:
        case EDGE:
            if (CC->subject_type == 0) {
                CC->subject_type = si;    // record if the ACT has a NODE or EDGE SUBJECT
            } else {
                if (si != CC->subject_type) {
                    if (si == NODE) {
                        emit_error(CC->context, si, "EDGE subject includes");
                    } else {
                        emit_error(CC->context, si, "NODE subject includes");
                    }
                }
            }
            if (*nextold) {
                if (si == (state_t) (*nextold)->state)    // old subject matches NODE or EDGE type
                {
                    // doesn't matter if old is shorter
                    // ... as long as no further substitutions are needed
                    nextoldelem = (*nextold)->u.list.first;    // in the recursion, iterate over
                    // the parts of the NODE or EDGE 
                    *nextold = (*nextold)->next;    // at this level, continue over the NODES or EDGES
                } else    // else we have no old, just ignore it
                {
                    nextoldelem = NULL;
                    *nextold = NULL;
                }
            }
            je_sameas_r(CC, elem, &nextoldelem, &object);    // recurse, adding result to a sublist
            new = move_list(C, &object);
            new->state = si;
            append_list(newlist, new);
            break;
        case NODEID:
            new = ref_list(C, elem);
            append_list(newlist, new);
            if (*nextold) {    // doesn't matter if old is shorter
                // ... as long as no further substitutions are needed
                *nextold = (*nextold)->next;
            }
            break;
        case EQL:
            if (*nextold) {
                new = ref_list(C, *nextold);
                append_list(newlist, new);

                *nextold = (*nextold)->next;

                C->stat_sameas++;
            } else {
                emit_error(CC->context, si, "No corresponding object found for same-as substitution");
            }
            break;
        default:
            if (*nextold) {    // doesn't matter if old is shorter
                // ... as long as no further substitutions are needed
                nextoldelem = (*nextold)->u.list.first;    // for the recursion
                *nextold = (*nextold)->next;    // at this level, continue over the elems
            }
            je_sameas_r(CC, elem, &nextoldelem, &object);    // recurse, adding result to a sublist
            new = move_list(C, &object);
            new->state = si;
            append_list(newlist, new);
            break;
        }
        elem = elem->next;
    }
}
