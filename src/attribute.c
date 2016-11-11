/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <sys/types.h>
#include <assert.h>

#include "thread.h"
#include "merge.h"
#include "match.h"
#include "attribute.h"

/**
 * ATTRID and VALUE are stored in separate trees:
 * Both trees are sorted by ATTRID
 *
 * The ATTRID tree is common to all the CONTAINERs in a THREAD
 * This is to permit the FRAGLIST representing the ATTRID to
 * be inserted just once, on first use, and then shared by
 * reference from all NODEs or EDGEs that set a VALUE for that ATTRID.
 *
 * Every NODE or EDGE has its own ATTRID + VALUE tree, but sharing the
 * ATTRID from the single ATTRID tree in THREAD.
 *
 *     a NODE or EDGE's
 *         VALUE tree
 *          /      \
 *       /    \  /    \   key           first         first
 *                     .--------> ATTR--------> ABC -------- > FRAGLIST of VALUE
 *                                 |    (valu)
 *                                 |
 *         THREAD's                | next
 *         ATTRID tree             | (attrid)
 *          /      \               |
 *       /    \  /    \   key      v    first
 *                    .---------> ABC ---------> FRAGLIST of ATTRID
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
 


/**
 * Stash away all the attrid in the THREAD->attrid tree, removing any duplicates.
 *
 * FIXME - attrid tree should be stored in SESSION,  but need some mutex to 
 *               make that safe when we have multiple threads
 *
 * FIXME - need something to keep track of how many elems are in attrid tree  .....  tough!
 *
 * @param CONTAINER - the current container context
 * @param attributes - the ATTRIBUTES branch
 */
void attrid_merge(CONTAINER_t * CONTAINER, elem_t * attributes)
{
    THREAD_t *THREAD = CONTAINER->THREAD;
    elem_t *attr, *attrid; 

    assert(attributes);
    assert((state_t)attributes->state == ATTRIBUTES);
    attr = attributes->u.l.first;
    while (attr) {
        assert((state_t)attr->state == ATTR);
        attrid = attr->u.l.first;
        assert(attrid);
        assert((state_t)attrid->state == ATTRID);


        THREAD->attrid =
            insert_item(LIST(), THREAD->attrid, &(attrid->u.l.first), merge_key);
        attr = attr->u.l.next;
    }
}



void value_merge(CONTAINER_t * CONTAINER, elem_t * attributes)
{
    THREAD_t *THREAD = CONTAINER->THREAD;
    elem_t *attr, *attrid, *attrid_str, *value, *value_str;

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
P(attrid_str);

        value = attr->u.l.first;
        if (value) {
            assert((state_t)value->state == VALUE);

            value_str = value->u.l.first;
P(value_str);
        }

        attr = attr->u.l.next;
    }
}
