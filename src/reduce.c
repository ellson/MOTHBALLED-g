/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "thread.h"
#include "merge.h"
#include "reduce.h"

void reduce(CONTAINER_t * CONTAINER, elem_t *list)
{
    THREAD_t * THREAD = CONTAINER->THREAD;
    state_t si, subjtype = 0;
    elem_t *subject, *attributes = NULL, *disambig = NULL;

    assert(list);

//E();
P(list);

    elem_t * elem = list->u.l.first;
    assert(elem); // must always be a subject
    si = (state_t) elem->state;
    switch (si) {
    case NODE:
    case SIS:
        subject = elem;
        subjtype = si; // NODE or SIS
        break;
    case EDGE:
        subject = elem->u.l.first;  // ENDPOINTS (legs)
        disambig = subject->u.l.next; // DISAMBIG (may be NULL)
        subjtype = si; // EDGE
        break;
    default:
        S(si);
        assert(0); // SUBJECT must be NODE,SIS, or EDGE
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
    }

    switch (subjtype) {
    case NODE:
        CONTAINER->nodes =
            insert_item(LIST(),
                CONTAINER->nodes,
                subject->u.l.next, // skip NODEID
                merge_attributes, NULL); 
        break;
    case EDGE:
        CONTAINER->edges =
            insert_item(LIST(),
                CONTAINER->edges,
                subject, // EDGES can have complex structure in SUBJECT
                merge_attributes, NULL); 
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
