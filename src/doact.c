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
 * @param act - the input ACT.
 * @return success/fail
 */
success_t doact(CONTAINER_t *CONTAINER, elem_t *act)
{
    THREAD_t *THREAD = CONTAINER->THREAD;
    elem_t *attributes;

    assert(act);
    assert(act->u.l.first);  // minimaly, an ACT must have a SUBJECT

    CONTAINER->stat_inactcount++;

#if 0
    printf("doact(): sameas=%d subj_has_ast=%d attr_has_ast=%d has_mum=%d has_node=%d has_edge=%d verb=%d\n",
            CONTAINER->has_sameas,
            CONTAINER->subj_has_ast,
            CONTAINER->attr_has_ast,
            CONTAINER->has_mum,
            CONTAINER->has_node,
            CONTAINER->has_edge,
            CONTAINER->verb);
#endif
//P(act);

    // perform SAMEAS substitutions
    sameas(CONTAINER, act);

    // FIXME - do rotate inputstream here
    //           we have a complete act
    //           associate sameas' previous, and the open inbuf inbuf, with each stream
    //           parse errors are associated with a particular stream.
    //              other streams should be able to continue

    // grammar ensures SUBJECT is purely NODE or EDGE, but we may not
    // know which until after sameas.   e.g.   <a b> =
    // just asserting that at this point we do know
    assert((CONTAINER->has_node && !CONTAINER->has_edge)
            || (!CONTAINER->has_node && CONTAINER->has_edge));

    // merge attrid in this ACT with tree of all attrid, keeping srings just once
    attributes = act->u.l.first->u.l.next;
    if (attributes) {
        assert((state_t)attributes->state == ATTRIBUTES);
        attrid_merge(CONTAINER, attributes);
    }

    // if ACT is a pattern the stash away for matching against later ACTs
    if (CONTAINER->subj_has_ast) {
        pattern_update(CONTAINER, act);
        return SUCCESS;
    }

#if 0
    new = pattern(CONTAINER, newact, CONTAINER->verb);
    free_list(LIST(), newact);
    if (new) {
       newact = new;
    } else {
        return SUCCESS;  // new pattern stored,  no more procesing for this ACT
    }

    //  N.B. (there can be multiple subjects after pattern subst.  Each matched
    //  pattern generates an additional subject.
#endif
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

//    free_list(LIST(), newact);

    return SUCCESS;
}
