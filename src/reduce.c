/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "reduce.h"

void reduce(CONTAINER_t * CONTAINER, elem_t *list)
{
    GRAPH_t * GRAPH = CONTAINER->GRAPH;
    LIST_t *LIST = (LIST_t*)GRAPH;
    state_t si, verb = 0, subjtype = 0;
    elem_t *elem, *subject, *attributes = NULL, *disambig = NULL;

    assert(list);

//E();
P(list);

    elem = list->u.l.first;
    assert(elem); // must always be a subject
    si = (state_t) elem->state;
    if (! (elem->u.l.first)) { // is the first elem just a tag?
        switch (si) {
        case QRY:
        case TLD:
            verb = si; // override default: 0 = add
            break;
        default:
            S(si);
            assert(0); // verb must be query (QRY),  or delete (TLD), (or add if no verb)
            break;
        }
        elem = elem->u.l.next;
    }
   
    si = (state_t) elem->state;
    switch (si) {
    case NODE:
    case SIBLING:
        subject = elem;
        subjtype = si; // NODE or SIBLING
        break;
    case EDGE:
        subject = elem->u.l.first;  // ENDPOINTS (legs)
        disambig = subject->u.l.next; // DISAMBIG (may be NULL)
        subjtype = si; // EDGE
        break;
    default:
        S(si);
        assert(0); // SUBJECT must be NODE,SIBLING, or EDGE
        break;
    }
    elem = elem->u.l.next;

    if (elem) {
        si = (state_t) elem->state;
        switch (si) {
        case ATTRIBUTES:
            attributes = elem;
            break;
        default:
            S(si);
            assert(0); // that should be all
            break;
        }
        elem = elem->u.l.next;
    }
    assert(elem == NULL);

    switch (subjtype) {
    case NODE:
        CONTAINER->nodes = insert_item(LIST, CONTAINER->nodes, subject->u.l.first); // skip NODEID
        break;
    case SIBLING:
        CONTAINER->nodes = insert_item(LIST, CONTAINER->nodes, subject->u.l.first->u.l.first); // skip NODEREF NODEID
        break;
    case EDGE:
        CONTAINER->edges = insert_item(LIST, CONTAINER->edges, subject); 
        break;
    default:
        S(subjtype);
        assert(0); // that should be all
        break;
    }
    if (disambig) {
        //FIXME - what to do with this?
    }
    if (attributes) {
        //FIXME - what to do with this?
    }

//E();
}
