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
#include "compare.h"
#include "attribute.h"

/**
 * ATTRID and VALUE are stored in separate trees:
 *
 * The ATTRID is an identifier, which are stored in a common tree to eliminate duplicates.
 *
 * Every NODE or EDGE has its own ATTRID + VALUE tree, but sharing the
 * ATTRID from the single identifier tree.
 *
 *     a NODE or EDGE's
 *         VALUE tree
 *          /      \
 *       /    \  /    \   key           first         first
 *                     .--------> ATTR--------> ABC -------- > FRAGLIST of VALUE
 *                                 |    (valu)
 *                                 |
 *         THREAD's                | next
 *      IDENTIFIER tree            | (attrid)
 *          /      \               |
 *       /    \  /    \   key      v    first
 *                    .---------> ABC ---------> FRAGLIST of identifier
 *
 * ( The "ABC---->FRAGLIST", which is a minimum of two elem_t
 * is optimized to a single SHORTSTRELEM elem_t whenever possible.
 *
 * So, typical cost is:  2 elem_t per unique ATTRID in a THREAD,
 * plus 3 elem_t per ATTRRID + VALUE pair NODE or EDGE
 *
 * elem_t are 32bytes on 64bit hosts, and 20bytes on 32bit. )
 *
 * In the single inact from the parser, we have an ATTRIBUTES list which
 * applies to all the NODES. or EDGES, from the SUBJECT
 * (which are all of the same type).  At this point we merge
 * the ATTRID into the THREAD's ATTRID tree, adding a reference
 * to the *old* version of any matching ATTRID, and freeing any
 * the new one if it is a duplicate.
 * The current ATTRIBUTES are modified to use the string from the THREAD's
 * ATTRID tree.
 *
 * After various transformations of the SUBJECT we get a list of individual
 * NODES or EDGES.  Then, the VALUES from the rewritten ATTRIBUTES
 * get merged with any existing VALUES used by the object.  The latest
 * VALUE list is saved in the NODE or EDGE's VALUE tree,
 * but referring to the ATTRID from the THREAD's ATTRID tree.
 */
 


void value_merge(CONTAINER_t * CONTAINER, elem_t * act)
{
    THREAD_t *THREAD = CONTAINER->THREAD;

P(act);

    elem_t *attributes = act->u.l.first->u.l.next;
    assert(attributes);
    assert((state_t)attributes->state == ATTRIBUTES);

    elem_t *attr = attributes->u.l.first;
    while (attr) {
        assert((state_t)attr->state == ATTR);

        // get pointers to the fraglists for the ATTRID and VALUE
        elem_t *attrid = attr->u.l.first;
        assert(attrid);
        assert((state_t)attrid->state == ATTRID);

        elem_t *attrid_str = attrid->u.l.first;
P(attrid_str);

        elem_t *value = attr->u.l.first;
        if (value) {
            assert((state_t)value->state == VALUE);

            elem_t *value_str = value->u.l.first;
P(value_str);
        }

        attr = attr->u.l.next;
    }
}
