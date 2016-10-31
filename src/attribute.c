/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <sys/types.h>
#include <assert.h>

#include "attribute.h"

void attribute_update(CONTENT_t * CONTENT, elem_t * attributes, state_t verb)
{
    PARSE_t * PARSE = CONTENT->PARSE;
    LIST_t *LIST = (LIST_t*)PARSE;
    elem_t *attrid, *value, *elem;
    state_t si;

    // ATTRID are stored in a sorted list of all ATTRID encountered
    // VALUE are stored in a list sorted by ATTRID for this ACT
    if (attributes) {
        elem = attributes->u.l.first;
        while (elem) {
            si = (state_t)elem->state;
            switch (si) {
            case LBR:
            case RBR:
                break; // ignore
            case ATTR:
                attrid = elem->u.l.first->u.l.first;
                value = elem->u.l.last->u.l.last->u.l.first;
P(LIST, attrid);
P(LIST, value);
                // FIXME - need a version that keeps old on match
                if (CONTENT->subject_type == NODE) {
    //                    CONTENT->node_attrid = insert_item(LIST, CONTENT->node_attrid, attr);
                } else {
    //                    CONTENT->edge_attrid = insert_item(LIST, CONTENT->edge_attrid, attr);
                }
                break;
            default:
                S(si);
                assert(0); // shouldn't be here
                break;
            }
            elem = elem->u.l.next;
        }
    }
}    
