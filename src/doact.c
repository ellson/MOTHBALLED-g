/* vim:set shiftwidth=4 ts=8 expandtab: */

/*************************************************************************
 * Copyright (c) 2017 AT&T Intellectual Property
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: John Ellson <john.ellson@gmail.com>
 *************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "thread.h"
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
    elem_t *activity;

// act initially points to the ACT tree from the parser.  We do a lot of
// referrencing of bits of that tree,  so we must be very careful with
// modifications to it to avoid retoactively affecting references from
// other parts.
//
// The initial ACT tree will be freed by the parser after we return

    assert(act);
    assert(act->u.l.first);    // minimmaly, an ACT must have a SUBJECT

//P(act);

    CONTAINER->stat_inactcount++;
    THREAD->stat_inactcount++;

    // replace each SAMEAS token in the current act, with reference to
    // the corresponding token in the previous act (which may itself
    // be a substituted sameas.)
    sameas(CONTAINER, act);

    // store pattern acts, or apply patterns to non-pattern acts
    //
    // patterns() also adds "_contenthash" (if any) to
    //     pattern and non-pattern acts.
    if (! (act = patterns(CONTAINER, act)) ) {
        return SUCCESS;   // new pattern stored, or removed (if no attributes).
    }
    // NB ACTs that are QRY or TLD may still have AST in SUBJECT
 
    // dispatch events for the ACT just finished
    //   the result is multiple simple acts -- hence activity
    activity = dispatch(CONTAINER, act, CONTAINER->verb, CONTAINER->has_mum);
    free_list(LIST(), act);

    act = activity->u.l.first;
    while (act) {
        THREAD->stat_outactcount++;
        CONTAINER->stat_outactcount++;
        // Populate GOM
        // eliminate redundancy by insertion-sorting into trees.
        reduce(CONTAINER, act, CONTAINER->verb);
        act = act->u.l.next;
    }

    // must free our rewritten act
    free_list(LIST(), activity);

    return SUCCESS;
}
