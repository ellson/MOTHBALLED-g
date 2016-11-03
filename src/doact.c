/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "doact.h"

success_t doact(CONTAINER_t *CONTAINER, elem_t *act)
{
    GRAPH_t * GRAPH = CONTAINER->GRAPH;
    LIST_t *LIST = (LIST_t*)GRAPH;
    elem_t *subject, *attributes, *elem;
    elem_t *newact, *newsubject, *newattributes;
    state_t verb = 0;

    assert(act);
    assert(act->u.l.first);  // minimaly, an ACT must have a SUBJECT

    GRAPH->stat_inactcount++;

//P(act);
    
    newact = new_list(LIST, ACT);

    // VERB has been recorded in GRAPH->verb during VERB exit processing 

//S(GRAPH->verb);

    subject = act->u.l.first;   // first item is SUBJECT
    assert(subject);
    assert(subject->state == (char)SUBJECT);

//====================== substitute sameas OBJECT(s) from previous SUBJECT
//P(subject)
    newsubject = sameas(CONTAINER, subject);
//P(subject);
    append_transfer(newact, newsubject);
//----------------------- example (from two consecutive ACTs)
// G:          <a b> <= c>
//
// subject:    SUBJECT OBJECT EDGE LEG ENDPOINT SIBLING NODEREF NODEID ABC a
//                                 LEG ENDPOINT SIBLING NODEREF NODEID ABC b
//             SUBJECT OBJECT EDGE LEG EQL
//                                 LEG ENDPOINT SIBLING NODEREF NODEID ABC c
//
//
// newsubject: SUBJECT OBJECT EDGE LEG ENDPOINT SIBLING NODEREF NODEID ABC a
//                                 LEG ENDPOINT SIBLING NODEREF NODEID ABC b
//             SUBJECT OBJECT EDGE LEG ENDPOINT SIBLING NODEREF NODEID ABC a
//                                 LEG ENDPOINT SIBLING NODEREF NODEID ABC c
//----------------------- 

//====================== stash ATTRID - there should be no structural change
//                                      to ATTRIBUTES,  just the STRING
//                                      represent the ATTRID stored only once

    attributes = subject->u.l.next;   // second item, if any, is attributes
    if (attributes) {
        assert(attributes->state == (char)ATTRIBUTES);
//P(attributes);
        newattributes = attrid_merge(CONTAINER, attributes);
//P(newattributes);
        append_transfer(newact, newattributes);
    }
//----------------------- example
// G:              a[foo=bar abc=xyz]
//
// attributes:     ATTRIBUTES ATTR ATTRID ABC foo
//                                 VALASSIGN VALUE ABC bar
//                            ATTR ATTRID ABC abc
//                                 VALASSIGN VALUE ABC xyz
//            
// newattribute:   ATTRIBUTES ATTR ATTRID ABC foo
//                                 VALASSIGN VALUE ABC bar
//                            ATTR ATTRID ABC abc
//                                 VALASSIGN VALUE ABC xyz
//----------------------- 

P(newact);

#if 0
//======================= collect, remove, or apply patterns
//
    new = pattern(CONTAINER, newact, verb);
    free_list(LIST, newact);
    if (new) {
       newact = new;
    } else {
        return SUCCESS;  // new pattern stored,  no more procesing for this ACT
    }

    //  N.B. (there can be multiple subjects after pattern subst.  Each matched
    //  pattern generates an additional subject.

P(newact);

// FIXME so this is probably flawed - doesn't it need a loop?
    // dispatch events for the ACT just finished
    new = dispatch(CONTAINER, newsubject, verb);
    if (new) {
        free_list(LIST, newsubject);
        newsubject = new;
    }

    elem = newsubject->u.l.first;
    while (elem) {
        GRAPH->stat_outactcount++;
P(elem);
        reduce(CONTAINER, elem);  // eliminate redundancy by insertion sorting into trees.

        elem = elem->u.l.next;
    }
#endif
    
    free_list(LIST, newact);

    return SUCCESS;
}
