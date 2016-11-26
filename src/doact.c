/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "thread.h"
#include "attribute.h"
#include "pattern.h"
#include "dispatch.h"
#include "reduce.h"
#include "sameas.h"
#include "doact.h"

/**
 * Invoked from the graph parser as soon as the parser
 * completes an input ACT.
 *
 * Various ACT rewrites are performed, followed by
 * updates to an internal representaion of the graph
 * in which the latest state of NODES and EDGES is maintained
 * along with a merge of all the ATTRIBUES for those
 * NODEs and EDGEs.
 *
 * @param CONTAINER context
 * @param act - the ACT from parse()
 * @return success/fail
 */
success_t doact(CONTAINER_t *CONTAINER, elem_t *act)
{
    THREAD_t *THREAD = CONTAINER->THREAD;
    elem_t *activity, *elem;


// act initially points to the ACT tree from the parser.  We do a lot of referrencing
// of bits of that tree,  so we must be very careful with modifications to it to avoid
// retoactively affecting references, from other parts.
//
// The initial ACT tree will be freed by the parser after we return

    assert(act);
    assert(act->u.l.first);            // minimmaly, an ACT must have a SUBJECT

    CONTAINER->stat_inactcount++;
    THREAD->stat_inactcount++;

    // replace each SAMEAS token in the current act, with reference to the corresponding
    // token in the previous act (which may itself be a substituted sameas.)
    sameas(CONTAINER, act);

    // merge attrid in this ACT with tree of all attrid, keeping strings just once
    attrid_merge(CONTAINER, act);

    // store pattern acts, or apply patterns to non-pattern acts
    if (! (act = patterns(CONTAINER, act)) ) {
        return SUCCESS;   // new pattern stored or removed (if it had no attributes).
    }
    // NB ACTs that are QRY or TLD may still have AST in SUBJECT

//P(act);
//printg(act);

    // dispatch events for the ACT just finished
    //   the result is multiple simple acts -- hence activity
    activity = dispatch(CONTAINER, act, CONTAINER->verb, CONTAINER->has_mum);
    free_list(LIST(), act);

//P(activity);
printg(activity);

    elem = activity->u.l.first;
    while (elem) {
        THREAD->stat_outactcount++;
        CONTAINER->stat_outactcount++;

        act = elem->u.l.first;
P(act);
//        reduce(CONTAINER, act);  // eliminate redundancy by insertion sorting into trees.

        elem = elem->u.l.next;
    }

    // must free our rewritten act
    free_list(LIST(), activity);

    return SUCCESS;
}
