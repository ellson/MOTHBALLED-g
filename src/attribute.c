/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <sys/types.h>
#include <assert.h>

#include "attribute.h"


/**
 * ATTRID and VALUE are stored in separate trees:
 * ATTRID are stored in one tree, and VALUE is stored in another
 *
 * Both trees are sorted by ATTRID
 *
 * The ATTRID tree is common to all the CONTAINERs in a SESSION
 * This is to permit the FRAGLIST representing the ATTRID to
 * be inserted just once, on first use, and then shared by
 * reference from all NODEs or EDGEs that set a VALUE for that ATTRID.
 *
 * Every NODE and EDGE has its own VALUE tree
 *
 *         an OBJECT's
 *         VALUE tree
 *          /      \
 *       /    \  /    \   key           first
 *                     .--------> ABC -------- > FRAGLIST of string VALUE
 *                                 |
 *                                 |
 *         SESSION's               | next
 *         ATTRID tree             |
 *          /      \               |
 *       /    \  /    \   key      v    first
 *                    .---------> ABC ---------> FRAGLIST of string ATTRID
 *
 *
 * After parse, we have a SUBJECT which applies to one or more OBJECTs
 * of the same type (NODE or EDGE).
 *
 * After varios transformations of the SUBJECT we get a list of individual
 * OBJECTS and the the single set of ATTRIBUTES from the ACT
 * need to be merged individually for each OBJECT.
 */

void attribute_update(CONTENT_t * CONTENT, elem_t * attributes, state_t verb)
{
    PARSE_t * PARSE = CONTENT->PARSE;
    LIST_t *LIST = (LIST_t*)PARSE;
    elem_t *attr, *attrid, *valassign, *value;
    state_t si;

    assert(attributes);

    attr = attributes->u.l.first;
    while (attr) {
        assert((state_t)attr->state == ATTR);

        // get pointers to the fraglists for the ATTRID and VALUE
        attrid = attr->u.l.first->u.l.first;
P(attrid);
        valassign = attr->u.l.first->u.l.next;
        if (valassign) {
            value = valassign->u.l.first->u.l.first;
P(value);
        }
        //
        // FIXME - need a version that keeps old on match
        if (CONTENT->subject_type == NODE) {
//            CONTENT->node_attrid = insert_item(LIST, CONTENT->node_attrid, attr);
        } else {
//            CONTENT->edge_attrid = insert_item(LIST, CONTENT->edge_attrid, attr);
        }

        attr = attr->u.l.next;
    }
}
