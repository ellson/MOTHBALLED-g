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
#include <time.h>
#include <sys/types.h>
#include <assert.h>

#include "thread.h"
#include "tuple.h"
#include "rewrite.h"

/** 
 * Rewrite act with additional attributes
 *
 * @param CONTAINER context
 * @param act to be processed
 * @return replacement act
 */
elem_t * rewrite(CONTAINER_t *CONTAINER, elem_t *act)
{
    THREAD_t *THREAD = CONTAINER->THREAD;
    elem_t *subject, *attributes, *disambig, *disambid = NULL;
    elem_t *newact, *newsubject, *newattributes;

#if 0
    elem_t *newact2;
    state_t schema[] = {SUBJECT, DISAMBIG, ATTRIBUTES};
    size_t schema_sz = sizeof(schema)/sizeof(schema[0]);
#endif
 
    assert(act);
    subject = act->u.l.first;
    assert(subject);                // minimaly, an ACT must have a SUBJECT
    disambig = subject->u.l.next;   // may be NULL or may be ATTRIBUTES
    if (disambig && (state_t)disambig->state == DISAMBIG) {
        attributes = disambig->u.l.next;  // may be NULL
        disambid = disambig->u.l.first;
    } else {
        attributes = disambig;   // wasn't disambig
        disambig = NULL;
    }
    if (THREAD->contenthash[0]) {
        elem_t *newattr, *newattrid, *newvalue, *newident;
        // Build newattributes for "_contenthash=xxxxx"
        // FIXME - need a litle-language to simplify
        //     these constructions
        newattr = new_list(LIST(), ATTR);
        newattrid = new_list(LIST(), ATTRID);
        append_transfer(newattr, newattrid);
        newident = new_shortstr(LIST(), ABC, "_contenthash");
        append_transfer(newattrid, newident);
        newvalue = new_list(LIST(), VALUE);
        append_transfer(newattr, newvalue);
        newident = new_shortstr(LIST(), ABC, THREAD->contenthash);
        append_transfer(newvalue, newident);
        if (attributes) { // append to existing attibutes
            append_transfer(attributes, newattr);
        } else {
            newattributes = new_list(LIST(), ATTRIBUTES);
            append_transfer(newattributes, newattr);
            append_transfer(act, newattributes);
        }
    }
    THREAD->contenthash[0] = '\0';


    // Now we are going to build a rewritten ACT tree, with references
    // to various bits from the parser's tree,  but no changes to it.
    //
    // In particular,  we must use fresh ACT, SUBJECT, DISAMBIG, ATTRIBUTE lists.

    newact = new_list(LIST(), ACT);
    newsubject = new_list(LIST(), SUBJECT);
    append_addref(newsubject, subject->u.l.first);
    append_transfer(newact, newsubject);

    // append disambig, if any
    if (disambig) {
        elem_t *newdisambig = new_list(LIST(), DISAMBIG);
        append_addref(newdisambig, disambid);
        append_transfer(newact, newdisambig);
    }

    newattributes = NULL;
    // append current attr, if any, after pattern_match so that
    // attr from patterns can be over-ridden
    if (attributes) {
        if (!newattributes) {
            newattributes = new_list(LIST(), ATTRIBUTES);
        }
        if (attributes->u.l.first) { // elide [] 
            append_addref(newattributes, attributes->u.l.first);
        }
    }
    if (newattributes) {
        append_transfer(newact, newattributes);
    }


#if 0
    newact2 = tuple(LIST(), ACT, schema, schema_sz, subject, disambig, attributes);
P(newact2);
    free_list(LIST(), newact2);
P(newact);
#endif

    return newact;
}
