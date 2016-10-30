/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "reduce.h"

void reduce(CONTENT_t * CONTENT, elem_t *list)
{
    PARSE_t * PARSE = CONTENT->PARSE;
    LIST_t *LIST = (LIST_t*)PARSE;
    state_t liststate, verb = 0, subjtype = 0;
    elem_t *elem, *subject, *attributes = NULL, *disambig = NULL;

    assert(list);

//E(LIST);

    elem = list->u.l.first;
    assert(elem); // must always be a subject
    liststate = (state_t) elem->state;
    if (! (elem->u.l.first)) { // is the first elem just a tag?
        switch (liststate) {
        case QRY:
        case TLD:
            verb = liststate; // override default: 0 = add
            break;
        default:
            assert(0); // verb must be query (QRY),  or delete (TLD), (or add if no verb)
            break;
        }
        elem = elem->u.l.next;
    }
   
    liststate = (state_t) elem->state;
    switch (liststate) {
    case NODE:
    case SIBLING:
        subject = elem;
        subjtype = liststate; // NODE or SIBLING
        break;
    case EDGE:
        subject = elem->u.l.first;  // ENDPOINTS (legs)
        disambig = subject->u.l.next; // DISAMBIG (may be NULL)
        subjtype = liststate; // EDGE
        break;
    default:
        assert(0); // SUBJECT must be NODE,SIBLING, or EDGE
        break;
    }
    elem = elem->u.l.next;

    if (elem) {
        liststate = (state_t) elem->state;
        switch (liststate) {
        case ATTRIBUTES:
            attributes = elem;
            break;
        default:
            assert(0); // that should be all
            break;
        }
        elem = elem->u.l.next;
    }
    assert(elem == NULL);

    switch (subjtype) {
    case NODE:
        CONTENT->nodes = insert(LIST, CONTENT->nodes, subject->u.l.first); // skip NODEID
        break;
    case SIBLING:
        CONTENT->nodes = insert(LIST, CONTENT->nodes, subject->u.l.first->u.l.first); // skip NODEREF NODEID
        break;
    case EDGE:
        CONTENT->edges = insert(LIST, CONTENT->edges, subject->u.l.first); 
        break;
    default:
        assert(0); // that should be all
        break;
    }
    if (disambig) {
        //FIXME - what to do with this?
    }
    if (attributes) {
        //FIXME - what to do with this?
    }

//E(LIST);
}
