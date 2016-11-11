/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "thread.h"
#include "sameas.h"

static void
sameas_r(CONTAINER_t * CONTAINER, elem_t **current, elem_t *replacement);

/**
 * Rewrite SAMEAS elems in ACT's subject with corresponding elems from the subject
 * of the immediately preceeninf ACT.
 *
 * @param CONTAINER container context
 * @param act from the parser
 */
void sameas(CONTAINER_t * CONTAINER, elem_t *act)
{
    THREAD_t *THREAD = CONTAINER->THREAD;
    elem_t **subject;

    assert(act);
    subject = &(act->u.l.first);

    assert(*subject);
    assert((state_t)(*subject)->state == SUBJECT);
    assert (CONTAINER->previous_subject);
    assert((state_t)CONTAINER->previous_subject->state == SUBJECT);

//E();
//P(*subject);
    if (CONTAINER->sameas) {
        // traverse SUBJECT tree, replacing SAMEAS with corresponding elem from previous_subject
        sameas_r(CONTAINER, subject, CONTAINER->previous_subject);
    }

    free_list((LIST_t*)THREAD, CONTAINER->previous_subject);   // Free the previous_subject
    CONTAINER->previous_subject = *subject;         // The current subject (after SAMEAS replacement) becomes
                                                    //    the previous_subject for next time
    (*subject)->refs++;                             // Increase subject's reference count to account
//E();
//P(*subject);
}

/**
 * rewrite target list with any SAMEAS elements substituted from replacement list
 *
 * @param CONTAINER container context
 * @param target - the list being rewritten
 * @param replacement - the source of replacements
 */
static void
sameas_r(CONTAINER_t * CONTAINER, elem_t **target, elem_t * replacement)
{
    THREAD_t *THREAD = CONTAINER->THREAD;
    state_t si;
    elem_t *next;

    while (*target) {
        si = (state_t) (*target)->state;
        switch (si) {
            case NODE:
            case SIS:
            case MUM:
            case KID:
            case PORT:
            case DISAMBIG:
                // no need to recurse deeper
                break;
            case SAMEAS:
                if (replacement) {
                    next = (*target)->u.l.next;  // just replace the SAMEAS element in the chain
                    *target = replacement;
                    (*target)->u.l.next = next;
                    replacement->refs++;
                    CONTAINER->stat_sameas++;
                } else {
                    // e.g. :      a (b =)
                    token_error((TOKEN_t*)THREAD,
                            "No corresponding object found for sameas substitution", si);
                }
                break;
            case SUBJECT:
            case SET:
            case EDGE:
            case LEG:
            case ENDPOINTSET:
                // need to recurse further for potential SAMEAS
                // doesn't matter if old is shorter
                // ... as long as no further substitutions are needed
                sameas_r(CONTAINER, &((*target)->u.l.first), replacement?replacement->u.l.first:NULL);    // recurse
                break;
            default:
                S(si);
                assert(0);
                break;
        }
        target = &((*target)->u.l.next);
        if (replacement) {
            replacement = replacement->u.l.next;
        }
    }
}
