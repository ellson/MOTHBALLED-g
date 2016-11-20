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
#include "compare.h"  // for printlist()
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
    elem_t *new, *elem;


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

    // grammar ensures SUBJECT is purely NODE or EDGE, but we may not
    // know which until after sameas.   e.g.   <a b> =
    // assert that at this point we do know
    assert((CONTAINER->has_node && !CONTAINER->has_edge)
            || (!CONTAINER->has_node && CONTAINER->has_edge));

    // FIXME - I don't like relying on sameas() to know if a subject is nodes or edges.
    //   - I may want an option to bypass sameas() processing
    //   - Its not clean, using a side-effect
    //   - It maybe incomplete - e.g. should a pattern of '*' match only nodes
    //     or should it apply to both?   If only nodes,  then what pattern applies to all edges?
    //     ( <*> applies to all one-legged edges )
    //
    //     Proposed solution:  deal with the uncertainty.  e.g. ambiguous patterns
    //     save as both node and edge patterns.   Remove the assert.
    //
    //     Alternative,  make '*' mean all nodes, and '<**>' mean all edges.

    // merge attrid in this ACT with tree of all attrid, keeping strings just once
    attrid_merge(CONTAINER, act);

    // store pattern acts, or apply patterns to non-pattern acts
    if (! (act = patterns(CONTAINER, act)) ) {
        return SUCCESS;   // new pattern stored or removed (if it had no attributes).
    }
    // NB ACTs that are QRY or TLD may still have AST in SUBJECT

//P(act);
printlist(act);

#if 0
    // dispatch events for the ACT just finished
    new = dispatch(CONTAINER, act, CONTAINER->verb, CONTAINER->has_mum);
    if (new) {
        free_list(LIST(), act);
        act = new;
    }

    elem = act->u.l.first->u.l.first;
    while (elem) {
        THREAD->stat_outactcount++;
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
