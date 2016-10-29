/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "sameas.h"

static void
sameas_r(CONTENT_t * CONTENT, elem_t * subject, elem_t ** nextold, elem_t * newlist);

/**
 * rewrite subject into a newsubject
 * compare subject with oldsubject
 * substitue EQL in newsubject from corresponding member of
 *   oldsubject (or error if old not available)
 *      replace subject and oldsubject with newsubject
 *
 * @param CONTENT container context
 * @param psubject a subject tree from the parser (may be multiple object with same attributes)
 */
void sameas(CONTENT_t * CONTENT, elem_t **psubject)
{
    LIST_t * LIST = (LIST_t *)(CONTENT->PARSE);
    elem_t *subject, *oldsubject, *nextold, *newsubject;

E(LIST,"sameas1");

    newsubject = new_list(LIST, SUBJECT);

    oldsubject = CONTENT->subject;
    assert(oldsubject);

    nextold = oldsubject->u.l.first;

    assert(psubject);
    subject = *psubject;
    assert(subject);
    assert((state_t)subject->state == SUBJECT);
    CONTENT->subject_type = 0;

    // rewrite subject into newsubject with any EQL elements substituted from oldsubject
    sameas_r(CONTENT, subject, &nextold, newsubject);
    assert(newsubject->u.l.first);

    free_list(LIST, subject);      // free original subject ( although refs are retained in other lists )
    free_list(LIST, oldsubject);   // free the previous oldsubject

    CONTENT->subject = newsubject; // save the newsubject as oldsubject

    newsubject->u.l.first->refs++; // and increase its reference count
    *psubject = newsubject;        //    to also save as the rewritten current subject

E(LIST,"sameas2");
}

/**
 * rewrite subject into newlist with any EQL elements substituted from oldlist
 *
 * @param CONTENT container context
 * @param subject
 * @param nextold
 * @param newlist
 */
static void sameas_r(CONTENT_t * CONTENT, elem_t * subject, elem_t ** nextold, elem_t * newlist)
{
    PARSE_t * PARSE = CONTENT->PARSE;
    TOKEN_t * TOKEN = (TOKEN_t *)PARSE;
    LIST_t * LIST = (LIST_t *)PARSE;
    elem_t *elem, *new, *nextoldelem = NULL;
    elem_t *object;
    state_t si;

    assert (subject->type == (char)LISTELEM);

    elem = subject->u.l.first;
#if 0
if (*nextold) {
    PARSE->sep = ' ';
    print_list(stderr, *nextold, 0, &(PARSE->sep));
    fprintf(stderr,"\n");
    print_list(stderr, elem, 0, &(PARSE->sep));
    fprintf(stderr,"\n\n");
}    
#endif
    while (elem) {
        si = (state_t) elem->state;
        object = new_list(LIST, si);
        switch (si) {
        case NODE:
        case EDGE:
            if (CONTENT->subject_type == 0) {
                CONTENT->subject_type = si;    // record if the ACT has a NODE or EDGE SUBJECT
            } else {
                if (si != CONTENT->subject_type) {
                    // all members of the SUBJECT must be of the same type: NODE or EDGE
                    // (this is really a shortcut to avoid extra productions in the grammar)
                    if (si == NODE) {
                        token_error(TOKEN, si, "EDGE subject includes");
                    } else {
                        token_error(TOKEN, si, "NODE subject includes");
                    }
                }
            }
            if (*nextold) {
                if (si == (state_t) (*nextold)->state)    // old subject matches NODE or EDGE type
                {
                    // doesn't matter if old is shorter
                    // ... as long as no further substitutions are needed
                    nextoldelem = (*nextold)->u.l.first;    // in the recursion, iterate over
                    // the parts of the NODE or EDGE 
                    *nextold = (*nextold)->next;    // at this level, continue over the NODES or EDGES
                } else    // else we have no old, just ignore it
                {
                    nextoldelem = NULL;
                    *nextold = NULL;
                }
            }
            sameas_r(CONTENT, elem, &nextoldelem, object);    // recurse, adding result to a sublist
            append_list(newlist, object);
            break;
        case DISAMBIG:
        case NODEID:
            new = ref_list(LIST, elem);
            append_list(newlist, new);
            if (*nextold) {    // doesn't matter if old is shorter
                // ... as long as no further substitutions are needed
                *nextold = (*nextold)->next;
            }
            break;
        case EQL:
            if (*nextold) {
                new = ref_list(LIST, *nextold);
                append_list(newlist, new);
                *nextold = (*nextold)->next;
                PARSE->stat_sameas++;
            } else {
                token_error(TOKEN, si, "No corresponding object found for same-as substitution");
            }
            break;
        default:
            if (*nextold) {    // doesn't matter if old is shorter
                // ... as long as no further substitutions are needed
                nextoldelem = (*nextold)->u.l.first;    // for the recursion
                *nextold = (*nextold)->next;    // at this level, continue over the elems
            }
            sameas_r(CONTENT, elem, &nextoldelem, object);    // recurse, adding result to a sublist
            append_list(newlist, object);
            break;
        }
        free_list(LIST,object);
        elem = elem->next;
    }
}
