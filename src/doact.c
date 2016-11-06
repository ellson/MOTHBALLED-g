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

success_t doact(CONTAINER_t *CONTAINER, elem_t *act)
{
    GRAPH_t *GRAPH = (GRAPH_t*)CONTAINER;
    THREAD_t *THREAD = CONTAINER->THREAD;
    LIST_t *LIST = (LIST_t*)THREAD;
    elem_t *subject, *attributes;
    elem_t *newact, *newsubject, *newattributes;

    assert(act);
    assert(act->u.l.first);  // minimaly, an ACT must have a SUBJECT

    GRAPH->stat_inactcount++;

//P(act);
//---------------------- love this example
// G:     (<a b> <c:1 ^^d:2/e:3 f:4/g:5/h:7 (i:8 j:9)>`baz)[foo=bar bar=foo]
//
// act:   ACT SUBJECT EDGE LEG ENDPOINT SIS NODEID ABC a
//                         LEG ENDPOINT SIS NODEID ABC b
//                    EDGE LEG ENDPOINT SIS NODEID ABC c
//                                          PORTID ABC 1
//                         LEG ENDPOINT MUM
//                                      MUM
//                                      SIS NODEID ABC d
//                                          PORTID ABC 2
//                                      KID NODEID ABC e
//                                          PORTID ABC 3
//                         LEG ENDPOINT SIS NODEID ABC f
//                                          PORTID ABC 4
//                                      KID NODEID ABC g
//                                          PORTID ABC 5
//                                      KID NODEID ABC h
//                                          PORTID ABC 7
//                         LEG ENDPOINT SIS NODEID ABC i
//                                          PORTID ABC 8
//                             ENDPOINT SIS NODEID ABC j
//                                          PORTID ABC 9
//                         DISAMBIG DISAMBID ABC baz
//            ATTRIBUTES ATTR ATTRID ABC foo
//                            VALUE ABC bar
//                       ATTR ATTRID ABC bar
//                            VALUE ABC foo
//----------------------- 
    



    newact = new_list(LIST, ACT);

// VERB has been recorded in GRAPH->verb during VERB exit processing 
//S(GRAPH->verb);

    subject = act->u.l.first;   // first item is SUBJECT
    assert(subject);
    assert(subject->state == (char)SUBJECT);




//====================== substitute sameas LEGs, or NODEs from previous SUBJECT
//P(subject)
    newsubject = sameas(CONTAINER, subject);
//P(newsubject);
    append_transfer(newact, newsubject);
//----------------------- example of consecutive EDGE ACTs
// G:          <a b> <= c>
//
// subject:    SUBJECT EDGE LEG ENDPOINT SIS NODEID ABC a
//                          LEG ENDPOINT SIS NODEID ABC b
//             SUBJECT EDGE LEG EQL
//                          LEG ENDPOINT SIS NODEID ABC c
//
//
// newsubject: SUBJECT EDGE LEG ENDPOINT SIS NODEID ABC a
//                          LEG ENDPOINT SIS NODEID ABC b
//             SUBJECT EDGE LEG ENDPOINT SIS NODEID ABC a
//                          LEG ENDPOINT SIS NODEID ABC c
//----------------------- 
//----------------------- example of consecutive NODE ACTs
// G:          (a b) (= c)
//
// subject:    SUBJECT NODE NODEID ABC a
//                     NODE NODEID ABC b
//             SUBJECT NODE EQL
//                     NODE NODEID ABC c
//
// newsubject: SUBJECT NODE NODEID ABC a
//                     NODE NODEID ABC b
//             SUBJECT NODE NODEID ABC a
//                     NODE NODEID ABC c
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
//                                 VALUE ABC bar
//                            ATTR ATTRID ABC abc
//                                 VALUE ABC xyz
//            
// newattribute:   ATTRIBUTES ATTR ATTRID ABC foo
//                                 VALUE ABC bar
//                            ATTR ATTRID ABC abc
//                                 VALUE ABC xyz
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
