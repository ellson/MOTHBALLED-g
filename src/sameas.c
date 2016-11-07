/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "thread.h"
#include "sameas.h"

static void
sameas_r(CONTAINER_t * CONTAINER, elem_t * subject, elem_t ** nextold, elem_t * newlist);

/**
 * Replace subject with a newsubject in which all EQL have
 * beeb substitued from the previous_subject, and save the newsubject
 * as the previous_subject for next time.
 *
 * @param CONTAINER container context
 * @param subject a subject tree from the parser (may be multiple object with same attributes)
 * @return rewritten with samas ('=') substituted
 */
elem_t *
sameas(CONTAINER_t * CONTAINER, elem_t * subject)
{
    LIST_t * LIST = (LIST_t *)(CONTAINER->THREAD);
    elem_t *nextold, *newsubject;

//E();
//P(subject);

    assert(subject);
    assert((state_t)subject->state == SUBJECT);
    assert (CONTAINER->previous_subject);

    newsubject = new_list(LIST, SUBJECT);

    nextold = CONTAINER->previous_subject->u.l.first;
    CONTAINER->subject_type = 0;  // will be set by sameas_r()

    // rewrite subject into newsubject with any EQL elements
    // substituted from oldsubject
    sameas_r(CONTAINER, subject, &nextold, newsubject);
    assert(newsubject->u.l.first);

    free_list(LIST, CONTAINER->previous_subject);   // free the previous oldsubject
    CONTAINER->previous_subject = newsubject; // save the newsubject as oldsubject
    newsubject->refs++;            // and increase its reference count

//E();
//P(newsubject);

    return newsubject;        //    to also save as the rewritten current subject
}

/**
 * rewrite subject into newlist with any EQL elements substituted from oldlist
 *
 * @param CONTAINER container context
 * @param subject
 * @param nextold
 * @param newlist
 */
static void
sameas_r(CONTAINER_t * CONTAINER, elem_t * subject, elem_t ** nextold, elem_t * newlist)
{
    LIST_t * LIST = (LIST_t *)(CONTAINER->THREAD);
    elem_t *elem, *new, *nextoldelem = NULL;
    elem_t *object;
    state_t si;

//P(subject);
    assert (subject->type == (char)LISTELEM);

    elem = subject->u.l.first;
    while (elem) {
        si = (state_t) elem->state;
        object = new_list(LIST, si);
        switch (si) {
        case NODE:
        case EDGE:
            if (CONTAINER->subject_type == 0) {
                CONTAINER->subject_type = si;    // record if the ACT has a NODE or EDGE SUBJECT
            } else {
                if (si != CONTAINER->subject_type) {
                    // all members of the SUBJECT must be of the same type: NODE or EDGE
                    // (this is really a shortcut to avoid extra productions in the grammar)
                    if (si == NODE) {
                        token_error((TOKEN_t*)LIST,
                                "EDGE subject includes", si);
                    } else {
                        token_error((TOKEN_t*)LIST,
                                "NODE subject includes", si);
                    }
                }
            }
            if (*nextold) {
                if (si == (state_t) (*nextold)->state) {  // old subject matches NODE or EDGE type
                    // doesn't matter if old is shorter
                    // ... as long as no further substitutions are needed
                    nextoldelem = (*nextold)->u.l.first;    // in the recursion, iterate over
                    // the parts of the NODE or EDGE 
                    *nextold = (*nextold)->u.l.next;    // at this level, continue over the NODES or EDGES
                } else {  // else we have no old, just ignore it
                    nextoldelem = NULL;
                    *nextold = NULL;
                }
            }
            sameas_r(CONTAINER, elem, &nextoldelem, object);    // recurse, adding result to a sublist
            append_addref(newlist, object);
            break;
        case PORTID:
        case NODEID:
        case DISAMBIG:
        case ENDPOINT:
            new = ref_list(LIST, elem);
            append_transfer(newlist, new);
            if (*nextold) {    // doesn't matter if old is shorter
                // ... as long as no further substitutions are needed
                *nextold = (*nextold)->u.l.next;
            }
            break;
        case EQL:
            if (*nextold) {
                new = ref_list(LIST, *nextold);
                append_transfer(newlist, new);
                *nextold = (*nextold)->u.l.next;
                ((GRAPH_t*)CONTAINER)->stat_sameas++;
            } else {
                // e.g. :      a (b =)
                token_error((TOKEN_t*)LIST,
                        "No corresponding object found for same-as substitution", si);
            }
            break;
        default:
            if (*nextold) {    // doesn't matter if old is shorter
                // ... as long as no further substitutions are needed
                nextoldelem = (*nextold)->u.l.first;    // for the recursion
                *nextold = (*nextold)->u.l.next;    // at this level, continue over the elems
            }
            sameas_r(CONTAINER, elem, &nextoldelem, object);    // recurse, adding result to a sublist
            append_addref(newlist, object);
            break;
        }
        free_list(LIST,object);
        elem = elem->u.l.next;
    }
}
