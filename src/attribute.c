/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <sys/types.h>
#include <assert.h>

#include "attribute.h"


/**
 * ATTRID and VALUE are stored in separate trees:
 * Both trees are sorted by ATTRID
 *
 * The ATTRID tree is common to all the CONTAINERs in a SESSION
 * ( FIXME - what about the meta graph, which includes all the stats ...)
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
 * At parse, we have an ATTRIBUTES list which applies to all the OBJECTs from the SUBJECT,
 * (which are all of the same type, NODE or EDGE).  At this point we merge
 * the ATTRID into the SESSION's ATTRID tree, adding a reference to the *new* version of
 * any matching ATTRID, and freeing any older duplicate strings.
 * The current ATTRIBUTES are not modified. 
 *
 * After various transformations of the SUBJECT we get a list of individual
 * OBJECTS.  Then, the VALUES from the rewritten ATTRIBUTES get merged with any existing 
 * VALUES used by the object.  The latest VALUE list is saved in the OBJECT's VALUE trees,
 * but referring to the ATTRID from the SESSION's ATTRID tree.
 */
 


/**
 * @param CONTENT - the current container context
 * @param attributes - the ATTRIBUTES branch before merging ATTRID 
 * @return replacement attributes list
 */
elem_t * attrid_merge(CONTENT_t * CONTENT, elem_t * attributes)
{
    PARSE_t * PARSE = CONTENT->PARSE;
    LIST_t *LIST = (LIST_t*)PARSE;
    elem_t *attr, *attrid, *attrid_str, *newattributes, *newattr, *new;
    state_t si;

    assert(attributes);
    assert((state_t)attributes->state == ATTRIBUTES);

    newattributes = new_list(LIST, ATTRIBUTES);

    attr = attributes->u.l.first;
    while (attr) {
        assert((state_t)attr->state == ATTR);

        // get pointer to the fraglist for the ATTRID
        attrid = attr->u.l.first;

        assert(attrid);
        assert((state_t)attrid->state == ATTRID);

        attrid_str = attrid->u.l.first;
P(attrid_str);

        new = ref_list(LIST, attr);
        append_transfer(newattributes, new);    // FIXME not right yet ...

        attr = attr->u.l.next;
    }
    return newattributes;
}



void value_merge(CONTENT_t * CONTENT, elem_t * attributes)
{
    PARSE_t * PARSE = CONTENT->PARSE;
    LIST_t *LIST = (LIST_t*)PARSE;
    elem_t *attr, *attrid, *attrid_str, *valassign, *value, *value_str;
    state_t si;

    assert(attributes);
    assert((state_t)attributes->state == ATTRIBUTES);

    attr = attributes->u.l.first;
    while (attr) {
        assert((state_t)attr->state == ATTR);

        // get pointers to the fraglists for the ATTRID and VALUE
        attrid = attr->u.l.first;
        assert(attrid);
        assert((state_t)attrid->state == ATTRID);

        attrid_str = attrid->u.l.first;
//P(attrid_str);

        valassign = attr->u.l.first->u.l.next;
        if (valassign) {
            value = valassign->u.l.first;
            assert((state_t)value->state == VALUE);

            value_str = value->u.l.first;
P(value_str);
        }

        attr = attr->u.l.next;
    }
}
