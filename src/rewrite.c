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

    if (IO()->contenthash[0]) { // Build newattr for "_contenthash=xxxxx"
        elem_t *newattrid, *newvalue, *newidentifier, *newvstring;

        newidentifier = new_shortstr(LIST(), ABC, "_contenthash");
        newvstring = new_shortstr(LIST(), ABC, IO()->contenthash);

        state_t ATTRID_schema[] = {ATTRID, ABC};
        newattrid = TUPLE(ATTRID_schema, newidentifier);

        state_t VALUE_schema[] = {VALUE, ABC};
        newvalue = TUPLE(VALUE_schema, newvstring);

        state_t ATTR_schema[] = {ATTR, ATTRID, VALUE};
        newattr = TUPLE(ATTR_schema, newattrid, newvalue);

        IO()->contenthash[0] = '\0';
    }

    // Now we are going to build a rewritten ACT tree, with references
    // to various bits from the parser's tree,  but no changes to it.
    //
    // In particular,  we must use fresh ACT, SUBJECT, DISAMBIG, ATTRIBUTE lists.

    newsubject = new_list(LIST(), SUBJECT);
    append_addref(newsubject, subject->u.l.first);

    if (disambig) {
        newdisambig = new_list(LIST(), DISAMBIG);
        append_addref(newdisambig, disambid);
    }

    if (attributes || newattr) {
        newattributes = new_list(LIST(), ATTRIBUTES);
    }
    while (attributes) {  // This would support a grammar of multiple [] atribute blocks
                          // The current grammar only allows 0 or 1.
                          // The code is fine either way.
        if (attributes->u.l.first) { // elide [] 
            append_addref(newattributes, attributes->u.l.first);
        }
        attributes = attributes->u.l.next;
    }
    if (newattr) {
        append_transfer(newattributes, newattr);
    }

    state_t ACT_schema[] = {ACT, SUBJECT, DISAMBIG, ATTRIBUTES};
    newact = TUPLE(ACT_schema, newsubject, newdisambig, newattributes);

    return newact;
}
