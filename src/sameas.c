/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "sameas.h"

static void je_sameas_r(CONTENT_t * CONTENT, elem_t * subject, elem_t ** nextold, elem_t * newlist);

/**
 * rewrite subject into a newsubject
 * compare subject with oldsubject
 * substitue EQL in newsubject from corresponding member of
 *   oldsubject (or error if old not available)
 *      replace subject and oldsubject with newsubject
 *
 * @param CONTENT container context
 * @param subject a subject tree from the parser (may be multiple object with same attributes)
 */
void je_sameas(CONTENT_t * CONTENT, elem_t * subject)
{
    LIST_t * LIST = (LIST_t *)(CONTENT->PARSE);
    elem_t *newsubject, *oldsubject, *nextold;
    elem_t subject_rewrite = { 0 };

    subject_rewrite.refs = 1; // prevent deletion

    newsubject = &subject_rewrite;
    oldsubject = &(CONTENT->subject);
    nextold = oldsubject->u.l.first;

    assert(subject);
    assert((state_t)subject->state == SUBJECT);
    CONTENT->subject_type = 0;

    // rewrite subject into newsubject with any EQL elements substituted from oldsubject
    je_sameas_r(CONTENT, subject, &nextold, newsubject);

    free_list(LIST, subject);     // free original subject ( although refs are retained in other lists )
    free_list(LIST, oldsubject);    // free the previous oldsubject

    newsubject->state = SUBJECT;  
    *oldsubject = *newsubject;    // save the newsubject as oldsubject

    assert(newsubject->u.l.first);

    newsubject->u.l.first->refs++;    // and increase its reference count
    *subject = *newsubject; //    to also save as the rewritten current subject
}

/**
 * rewrite subject into newlist with any EQL elements substituted from oldlist
 *
 * @param CONTENT container context
 * @param subject
 * @param nextold
 * @param mewlist
 */
static void je_sameas_r(CONTENT_t * CONTENT, elem_t * subject, elem_t ** nextold, elem_t * newlist)
{
    PARSE_t * PARSE = CONTENT->PARSE;
    TOKEN_t * TOKEN = (TOKEN_t *)PARSE;
    LIST_t * LIST = (LIST_t *)PARSE;
    elem_t *elem, *new, *nextoldelem = NULL;
    elem_t object = { 0 };
    state_t si;

    object.refs = 1; // prevent deletion

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
                        je_token_error(TOKEN, si, "EDGE subject includes");
                    } else {
                        je_token_error(TOKEN, si, "NODE subject includes");
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
            je_sameas_r(CONTENT, elem, &nextoldelem, &object);    // recurse, adding result to a sublist
            new = move_list(LIST, &object);
            new->state = si;
            append_list(newlist, new);
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
                je_token_error(TOKEN, si, "No corresponding object found for same-as substitution");
            }
            break;
        default:
            if (*nextold) {    // doesn't matter if old is shorter
                // ... as long as no further substitutions are needed
                nextoldelem = (*nextold)->u.l.first;    // for the recursion
                *nextold = (*nextold)->next;    // at this level, continue over the elems
            }
            je_sameas_r(CONTENT, elem, &nextoldelem, &object);    // recurse, adding result to a sublist
            new = move_list(LIST, &object);
            new->state = si;
            append_list(newlist, new);
            break;
        }
        elem = elem->next;
    }
}
