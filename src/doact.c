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
 * @param branch - the ACT branch from parse()
 * @return success/fail
 */
success_t doact(CONTAINER_t *CONTAINER, elem_t *branch)
{
    THREAD_t *THREAD = CONTAINER->THREAD;
    elem_t *branch_subject, *branch_attributes;
    elem_t *act, *subject, *attributes;


// branch points to the ACT tree from the parser.  We do a lot of referrencing
// of bits of that tree,  so we must be very careful with modifications to it as they can
// retoactively affect the refences.
//
// sameas() replaces SAMEAS '=' tokens in the branch tree with references to the previous.
//
// attrd_merge() combines multiple uses of the same attrid string, into
// one, using multiple references to the same string and removing duplicates.
// It does this in the branch tree.
//
// pattern_update(), a pattern_remove() manage only references to the branch tree
// without modification.
//
// The branch tree will be freed by the parser after we retrn

    assert(branch);

    branch_subject = branch->u.l.first;
    assert(branch_subject);                // minimaly, an ACT must have a SUBJECT

    branch_attributes = branch_subject->u.l.next; // may be NULL

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
P(branch);
#endif

    // perform SAMEAS substitutions
    sameas(CONTAINER, branch);

    // grammar ensures SUBJECT is purely NODE or EDGE, but we may not
    // know which until after sameas.   e.g.   <a b> =
    // assert that at this point we do know
    assert((CONTAINER->has_node && !CONTAINER->has_edge)
            || (!CONTAINER->has_node && CONTAINER->has_edge));

    // merge attrid in this ACT with tree of all attrid, keeping srings just once
    if (branch_attributes) {
        assert((state_t)branch_attributes->state == ATTRIBUTES);
        attrid_merge(CONTAINER, branch_attributes);
    }

    // pattern_acts
    if ( !CONTAINER->verb) { // if verb is default (add)
        if (CONTAINER->subj_has_ast) {
            if (branch_attributes) {
                // if the pattern act has attributes, then it is added to saved patterns
                // (if if the pattern subject is already saved, then it is replaced
                // by this new one)
                pattern_update(CONTAINER, branch);
            } else {
                // if the pattern has no attributes then it is removed from saved patterns
                // N.B. This is how patterns are deleted
                pattern_remove(CONTAINER, branch);
            }
P(CONTAINER->node_patterns);
P(CONTAINER->edge_patterns);
            return SUCCESS;  // new pattern stored,  no more procesing for this ACT
        }
    }

P(branch);
// Now we are going to build a rewritten ACT tree, with references
// to various bits from the parser's tree,  but no changes to it.
//
// In particular,  we must use fresh ACT, SUBJECT, ATTRIBUTE lists.

    act = new_list(LIST(), ACT);
    subject = new_list(LIST(), SUBJECT);
    append_addref(subject, branch_subject->u.l.first);
    append_addref(act, subject);

    // append a new empty attributes lis
    attributes = new_list(LIST(), ATTRIBUTES);
    append_addref(act, attributes);

    // append pattern attrs, if any
    if ( !CONTAINER->verb) { // if verb is default (add)
          pattern_match(CONTAINER, act);
    }
    // append current attr, if any, after pattern_match so that
    // attr from patterns can be over-ridden
    if (branch_attributes) {
        append_addref(attributes, branch_attributes->u.l.first);
    }
    if (! attributes->u.l.first) {
        remove_next_from_list(LIST(), act, subject);  // remove empty attributes
        attributes->refs--;  // FIXME - why is this needed
    }
    
    // patterns now applied for "add"  verb - may now have multiple ACTs
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

    free_list(LIST(), act);
    free_list(LIST(), subject);
    free_list(LIST(), attributes);

    return SUCCESS;
}
