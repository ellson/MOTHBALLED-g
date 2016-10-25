/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <assert.h>

#include "libje_private.h"
#include "tree.h"
#include "frag.h"

// forward declaration
static void reduce_list_r(container_CONTEXT_t *CC, elem_t * list);
static void reduce_print_frags(state_t liststate, elem_t * elem, char *sep);
static void reduce_print_shortstr(elem_t * elem, char *sep);

void je_reduce(container_CONTEXT_t * CC, elem_t *list)
{
    CONTEXT_t *C = CC->C;
    LIST_t *LIST = (LIST_t*)C;
    elem_t * elem;
    state_t liststate;
    state_t verb = 0;
    state_t subjtype = 0;
    elem_t *subject, *attributes = NULL, *disambig = NULL;

printf("\n== ACT ==\n");
P(list);

    assert(list);
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
        elem = elem->next;
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
        disambig = subject->next; // DISAMBIG (may be NULL)
        subjtype = liststate; // EDGE
        break;
    default:
        assert(0); // SUBJECT must be NODE,SIBLING, or EDGE
        break;
    }
    elem = elem->next;

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
        elem = elem->next;
    }
    assert(elem == NULL);

    switch (subjtype) {
    case NODE:
        CC->nodes = insert(LIST, CC->nodes, subject->u.l.first); // skip NODEID
        break;
    case SIBLING:
//P(subject->u.l.first->u.l.first);
        CC->nodes = insert(LIST, CC->nodes, subject->u.l.first->u.l.first); // skip NODEREF NODEID
        break;
    case EDGE:
        CC->edges = insert(LIST, CC->edges, subject->u.l.first); 
        break;
    default:
        assert(0); // that should be all
        break;
    }
    if (disambig) {
        //FIXME - what to do with this?
    }
    if (attributes) {
//P(attributes);
        //FIXME - what to do with this?
    }
}
