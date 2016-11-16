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
    elem_t *subject, *attributes;


// act initially points to the ACT tree from the parser.  We do a lot of referrencing
// of bits of that tree,  so we must be very careful with modifications to it to avoid
// retoactively affecting references, from other parts.
//
// The initial ACT tree will be freed by the parser after we return

    assert(act);
    subject = act->u.l.first;
    assert(subject);                // minimaly, an ACT must have a SUBJECT
    attributes = subject->u.l.next; // may be NULL

    CONTAINER->stat_inactcount++;

    // replace each SAMEAS token in the current act, with reference to the corresponding
    // token in the previous act (which may itself be a substituted sameas.)
    sameas(CONTAINER, act);

    // grammar ensures SUBJECT is purely NODE or EDGE, but we may not
    // know which until after sameas.   e.g.   <a b> =
    // assert that at this point we do know
    assert((CONTAINER->has_node && !CONTAINER->has_edge)
            || (!CONTAINER->has_node && CONTAINER->has_edge));

    // merge attrid in this ACT with tree of all attrid, keeping strings just once
    attrid_merge(CONTAINER, attributes);

    // store pattern acts, or apply patterns to non-pattern acts
    if (! (act = patterns(CONTAINER, act)) ) {
        return SUCCESS;
    }
    // patterns now applied for "add"  verb 
    // may still have subj_has_ast in QRY or TLD

P(act);

#if 0
// FIXME so this is probably flawed - doesn't it need a loop?
    // dispatch events for the ACT just finished
    new = dispatch(CONTAINER, newsubject, CONTAINER->verb, CONTAINER->mum);
    if (new) {
        free_list(LIST(), newsubject);
        newsubject = new;
    }

    elem = newsubject->u.l.first;
    while (elem) {
        CONTAINER->stat_outactcount++;
P(elem);
        reduce(CONTAINER, elem);  // eliminate redundancy by insertion sorting into trees.

        elem = elem->u.l.next;
    }
#endif

    // must free our rewritten act
    free_list(LIST(), act);

    return SUCCESS;
}
