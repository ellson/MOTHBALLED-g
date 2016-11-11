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
 * Various ACT rewrites are performed in the function,
 * culminating in updates to an internal representation
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
// for debugging only

printf("doact(): sameas=%d pattern=%d mum=%d verb=%d\n", 
        CONTAINER->sameas,
        CONTAINER->pattern,
        CONTAINER->mum,
        CONTAINER->verb);

// VERB has already been extracted and is available as an arg to this func. 
if (CONTAINER->verb) { S(CONTAINER->verb); }

// The need for MUM's assistance with component(s) of the SUBJECT has been extracted
if (CONTAINER->mum) { S(CONTAINER->mum); }

// The ACT as provided by parse()
P(act);
#endif



//---------------------- love this example
// G:     (<a b> <c:1 ^^d:2/e:3 f:4/g:5/h:7 (i:8 j:9)>`baz)[foo=bar bar=foo]
//
// act:   ACT SUBJECT EDGE LEG SIS NODEID ABC a
//                         LEG SIS NODEID ABC b
//                    EDGE LEG SIS NODEID ABC c
//                                 PORTID ABC 1
//                         LEG MUM
//                             MUM
//                             SIS NODEID ABC d
//                                 PORTID ABC 2
//                             KID NODEID ABC e
//                                 PORTID ABC 3
//                         LEG SIS NODEID ABC f
//                                 PORTID ABC 4
//                             KID NODEID ABC g
//                                 PORTID ABC 5
//                             KID NODEID ABC h
//                                 PORTID ABC 7
//                         LEG SIS NODEID ABC i
//                                 PORTID ABC 8
//                             SIS NODEID ABC j
//                                 PORTID ABC 9
//                         DISAMBIG DISAMBID ABC baz
//            ATTRIBUTES ATTR ATTRID ABC foo
//                            VALUE ABC bar
//                       ATTR ATTRID ABC bar
//                            VALUE ABC foo
//----------------------- 
    



//====================== substitute SAMEAS

    sameas(CONTAINER, act);

//P(act);
//----------------------- example of consecutive EDGE ACTs
// G:          <a b> <= c>
//
// before:  ACT SUBJECT EDGE LEG SIS NODEID ABC a
//                           LEG SIS NODEID ABC b
//          ACT SUBJECT EDGE LEG SAMEAS EQL
//                           LEG SIS NODEID ABC c
//
// after:   ACT SUBJECT EDGE LEG SIS NODEID ABC a
//                           LEG SIS NODEID ABC b
//          ACT SUBJECT EDGE LEG SIS NODEID ABC a
//                           LEG SIS NODEID ABC c
//----------------------- 
//----------------------- example of consecutive NODE ACTs
// G:          (a b) (= c)
//
// before:  ACT SUBJECT NODE NODEID ABC a
//                      NODE NODEID ABC b
//          ACT SUBJECT NODE SAMEAS EQL
//                      NODE NODEID ABC c
//
// after:   ACT SUBJECT NODE NODEID ABC a
//                      NODE NODEID ABC b
//          ACT SUBJECT NODE NODEID ABC a
//                      NODE NODEID ABC c
//----------------------- 



//====================== stash ATTRID - there should be no structural change
//                                      to ATTRIBUTES,  just the STRING
//                                      represent the ATTRID stored only once

    attributes = act->u.l.first->u.l.next;   // second item, if any, is attributes
    if (attributes) {
        assert((state_t)attributes->state == ATTRIBUTES);
        attrid_merge(CONTAINER, attributes);
//P(THREAD->attrid);
    }
//----------------------- example
// G:              a[foo=bar abc=xyz]
//
// attributes:     ATTRIBUTES ATTR ATTRID ABC foo
//                                 VALUE ABC bar
//                            ATTR ATTRID ABC abc
//                                 VALUE ABC xyz
//            
// newattributes: (unchanged)
//----------------------- 

P(act);

#if 0
//======================= collect, remove, or apply patterns
//
    new = pattern(CONTAINER, newact, CONTAINER->verb);
    free_list(LIST(), newact);
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
