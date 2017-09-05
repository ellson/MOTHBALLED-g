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
    elem_t *newact, *newsubject, *newdisambig = NULL, *newattr = NULL, *newattributes = NULL;

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

    if (THREAD->contenthash[0]) { // Build newattr for "_contenthash=xxxxx"
        elem_t *newattrid, *newvalue, *newidentifier, *newvstring;

        newidentifier = new_shortstr(LIST(), ABC, "_contenthash");
        newvstring = new_shortstr(LIST(), ABC, THREAD->contenthash);

        state_t ATTRID_schema[] = {ATTRID, ABC};
        newattrid = TUPLE(ATTRID_schema, newidentifier);

        state_t VALUE_schema[] = {VALUE, ABC};
        newvalue = TUPLE(VALUE_schema, newvstring);

        state_t ATTR_schema[] = {ATTR, ATTRID, VALUE};
        newattr = TUPLE(ATTR_schema, newattrid, newvalue);

#if 1
// FIXME - can't we do this below now?
        if (attributes) { // append to existing attibutes
            append_transfer(attributes, newattr);
        } else {
            state_t ATTRIBUTES_schema[] = {ATTRIBUTES, ATTR};
            newattributes = TUPLE(ATTRIBUTES_schema, newattr);

            append_transfer(act, newattributes);
        }
#endif
        THREAD->contenthash[0] = '\0';
    }
    newattributes = NULL;


    // Now we are going to build a rewritten ACT tree, with references
    // to various bits from the parser's tree,  but no changes to it.
    //
    // In particular,  we must use fresh ACT, SUBJECT, DISAMBIG, ATTRIBUTE lists.

    newsubject = new_list(LIST(), SUBJECT);
    append_addref(newsubject, subject->u.l.first);

    // append disambig, if any
    if (disambig) {
        newdisambig = new_list(LIST(), DISAMBIG);
        append_addref(newdisambig, disambid);
    }

    // append current attr, if any, after template_match so that
    // attr from templates can be over-ridden
    if (attributes) {
        if (!newattributes) {
            newattributes = new_list(LIST(), ATTRIBUTES);
        }
        if (attributes->u.l.first) { // elide [] 
            append_addref(newattributes, attributes->u.l.first);
        }
    }

    state_t ACT_schema[] = {ACT, SUBJECT, DISAMBIG, ATTRIBUTES};
    newact = TUPLE(ACT_schema, newsubject, newdisambig, newattributes);

    return newact;
}
