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
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "thread.h"
#include "sameas.h"

static void
sameas_r(CONTAINER_t * CONTAINER, elem_t *current, elem_t *replacement);

/**
 * Rewrite SAMEAS elems in ACT's SUBJECT from the
 * corresponding elems from the previous SUBJECT.
 *
 * @param CONTAINER container context
 * @param act from the parser
 */
void sameas(CONTAINER_t * CONTAINER, elem_t *act)
{
    THREAD_t *THREAD = CONTAINER->THREAD;
    elem_t *subject;

    assert(act);
    subject = act->u.l.first;

    assert(subject);
    assert((state_t)subject->state == SUBJECT);
    assert (CONTAINER->previous);
    assert((state_t)CONTAINER->previous->state == SUBJECT);

    if (CONTAINER->has_sameas) {
        // traverse SUBJECT tree, replacing SAMEAS
        //   with corresponding elem from previous
        sameas_r(CONTAINER, subject, CONTAINER->previous);
    }

    free_list(LIST(), CONTAINER->previous);
    CONTAINER->previous = subject;
    subject->refs++;
}

/**
 * rewrite target list with any SAMEAS elements substituted from replacement list
 *
 * @param CONTAINER container context
 * @param target - the list being rewritten
 * @param replacement - the source of replacements
 */
static void
sameas_r(CONTAINER_t * CONTAINER, elem_t *target, elem_t * replacement)
{
    THREAD_t *THREAD = CONTAINER->THREAD;
    state_t si;

    while (target) {
        si = (state_t) target->state;
        switch (si) {
            case NODE:
            case SIS:
            case MUM:
            case KID:
            case PORT:
            case DISAMBIG:
            case ATTRIBUTES:
                // no need to recurse deeper
                break;
            case SAMEAS:
                if (replacement) {
                    // just overwrite SAMEAS element
 
                    // FIXME  - Ha! Look how ugly this is - and difficult to get right!
                    
                    if (target->u.l.first) { // in case we still have EQL
                        free_list(LIST(), target->u.l.first);
                    }
                    if ( (target->u.l.first = replacement->u.l.first) ) {
                        replacement->u.l.first->refs++;   // increment refs
                    }
                    target->u.l.last = replacement->u.l.last;
                    target->len = replacement->len;
                    // keep u.l.next from SAMEAS
                    // type should be LISTELEM, don't bother to copy
                    target->state = replacement->state;

                    // we may not be able to tell if an ACT with SAMEAS is a NODE or
                    //   EDGE subject,  until we substitute from previous
                    //   e.g.     (a b) =
                    if (target->state == NODE || target->u.l.first->state == NODE) {
                        CONTAINER->has_node = NODE;
                    }
                    if (target->state == EDGE || target->u.l.first->state == EDGE) {
                        CONTAINER->has_edge = EDGE;
                    }
                    CONTAINER->stat_sameas++;
                } else {
                    // e.g. :      a (b =)
                    token_error((TOKEN_t*)THREAD,
                        "No corresponding object found for sameas substitution", si);
                }
                break;
            case SUBJECT:
            case SET:
            case EDGE:
            case LEG:
            case ENDPOINTSET:
                // need to recurse further for potential SAMEAS
                // doesn't matter if old is shorter
                // ... as long as no further substitutions are needed
                sameas_r(CONTAINER, target->u.l.first, replacement?replacement->u.l.first:NULL);    // recurse
                break;
            default:
                S(si);
                assert(0);
                break;
        }
        target = target->u.l.next;
        if (replacement) {
            replacement = replacement->u.l.next;
        }
    }
}
