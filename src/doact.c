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
    elem_t *subject, *attributes, *newact, *newsubject, *newattributes = NULL, *elem;
    state_t verb = 0;

    assert(act);
    assert(act->u.l.first);  // minimaly, an ACT must have a SUBJECT

    PARSE->stat_inactcount++;

//P(act);

    // VERB has been recorded in PARSE->verb during VERB exit processing 

//S(PARSE->verb);

    subject = act->u.l.first;   // first item is SUBJECT
    assert(subject);
    assert(subject->state == (char)SUBJECT);

//====================== substitute sameas OBJECT(s) from previous SUBJECT
//P(subject)
    newsubject = sameas(CONTENT, subject);
//P(subject);
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

//====================== stash ATTRID - should be no structural change to ATTRIBUTES
//                                      just the STRING represent the ATTRID stored only once

    attributes = subject->u.l.next;   // second item, if any, is attributes
    if (attributes) {
        assert(attributes->state == (char)ATTRIBUTES);
P(attributes);
        newattributes = attrid_merge(CONTENT, attributes);
P(newattributes);
    }
//----------------------- example
// G:       a[foo=bar abc=xyz]
//
// attributes:
//
// newattributes:
//----------------------- 

//    newact = new_list(LIST, ACT);
//    append_transfer(newact, newsubject);
//    append_addref(newact, attributes);
//
//P(newact);

#if 0
//======================= collect, remove, or apply patterns
//
    new = pattern(CONTENT, newsubject, verb);
    free_list(LIST, newsubject);
    if (new) {
        newsubject = new;
    } else {
        return SUCCESS;  // new pattern stored,  no more procesing for this ACT
    }

    //  N.B. (there can be multiple subjects after pattern subst.  Each matched
    //  pattern generates an additional subject.

    assert(newsubject);
//P(newsubject);

// FIXME so this is probably flawed - doesn't it need a loop?
    // dispatch events for the ACT just finished
    new = dispatch(CONTENT, newsubject, verb);
    if (new) {
        free_list(LIST, newsubject);
        newsubject = new;
    }

    elem = newsubject->u.l.first;
    while (elem) {
        PARSE->stat_outactcount++;
P(elem);
        reduce(CONTENT, elem);  // eliminate redundancy by insertion sorting into trees.

        elem = elem->u.l.next;
    }
#endif
    
    free_list(LIST, newsubject);
    if (newattributes) {
        free_list(LIST, newattributes);
    }

    return SUCCESS;
}
