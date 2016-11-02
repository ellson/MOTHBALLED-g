/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "doact.h"

success_t doact(CONTENT_t *CONTENT, elem_t *act)
{
    PARSE_t * PARSE = CONTENT->PARSE;
    LIST_t *LIST = (LIST_t*)PARSE;
    elem_t *subject, *attributes, *new, *newsubjects, *elem;
    state_t verb = 0;

    assert(act);
    assert(act->u.l.first);  // minimaly, an ACT must have a SUBJECT

    PARSE->stat_inactcount++;

P(act);

    // VERB has been recorded in PARSE->verb during VERB exit processing 
//S(PARSE->verb);

    // sameas substitutions have been performed on the subject
 

    subject = act->u.l.first;   // first item is SUBJECT
    assert(subject);
    assert(subject->state == (char)SUBJECT);

//======================= process ATTRIBUTES (if any)
// Do this after sameas so we have the NODE/EDGE classification
// Later we reattach the attributes to the reassembled ACTs
//
    attributes = subject->u.l.next;  // second is ATTRIBUTES
    if (attributes) {
        attribute_update(CONTENT, attributes, verb);
    }




//P(newsubjects);
//----------------------- example
// G:       a[foo=bar abc=xyz]
//
// Parse:   ACT SUBJECT OBJECT NODE NODEID ABC a
//              ATTRIBUTES LBR
//                         ATTR ATTRID ABC foo
//                              VALASSIGN EQL
//                                        VALUE ABC bar
//                         ATTR ATTRID ABC abc
//                              VALASSIGN EQL
//                                        VALUE ABC xyz
//                         RBR
//
// Here:    SUBJECT OBJECT NODE NODEID ABC a
//----------------------- 


#if 0
//======================= collect, remove, or apply patterns
//
    new = pattern(CONTENT, newsubjects, verb);
    free_list(LIST, newsubjects);
    if (new) {
        newsubjects = new;
    } else {
        return SUCCESS;  // new pattern stored,  no more procesing for this ACT
    }

    //  N.B. (there can be multiple subjects after pattern subst.  Each matched
    //  pattern generates an additional subject.

    assert(newsubjects);
//P(newsubjects);

// FIXME so this is probably flawed - doesn't it need a loop?
    // dispatch events for the ACT just finished
    new = dispatch(CONTENT, newsubjects, verb);
    if (new) {
        free_list(LIST, newsubjects);
        newsubjects = new;
    }

    elem = newsubjects->u.l.first;
    while (elem) {
        PARSE->stat_outactcount++;
P(elem);
        reduce(CONTENT, elem);  // eliminate redundancy by insertion sorting into trees.

        elem = elem->u.l.next;
    }
#endif

    return SUCCESS;
}
