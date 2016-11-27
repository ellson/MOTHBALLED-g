/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "thread.h"
#include "reduce.h"


/**
 * merge ATTRIBUTEs for a NODE or EDGE which may have existing ATTRIBUTES
 *
 * @param LIST 
 * @param subject - the new attributes
 * @param key     - the key with previous subject
 * @return        - the key with previous subject  - not used
 */
static elem_t * merge_attributes(LIST_t *LIST, elem_t *subject, elem_t *key)
{
    elem_t *disambig, *attributes;
   
    disambig = subject->u.l.next;
    if (disambig) {
        if ((state_t)disambig->state == DISAMBIG) {
            attributes = disambig->u.l.next;
        } else {
            attributes = disambig;
            disambig = NULL;
        }
    } 
    if (attributes) {
        assert((state_t)attributes->state == ATTRIBUTES);
        elem_t *attr = attributes->u.l.first;

        // FIXME - insert new values into attribute tree - new value wins
        while (attr) {

//printf("hello\n");

            attr = attr->u.l.next;
        }
    }
    return key;
}

void reduce(CONTAINER_t * CONTAINER, elem_t *act, state_t verb)
{
    THREAD_t * THREAD = CONTAINER->THREAD;
    elem_t *subject;

    assert(act);
    assert((state_t)act->state == ACT);

    subject = act->u.l.first;
    assert(subject);
    assert((state_t)subject->state == SUBJECT);

    assert(subject->u.l.first);

    // FIXME - attributes need to be in a sorted tree

    switch ((state_t)subject->u.l.first->state) {
    case NODE:
        if (!verb) {
            CONTAINER->nodes = insert_item(LIST(), CONTAINER->nodes, subject,
                merge_attributes, NULL); 
        } else if (verb == TLD) {
            CONTAINER->nodes = remove_item(LIST(), CONTAINER->nodes, subject);
        }
        break;
    case EDGE:
        if (!verb) {
            CONTAINER->edges = insert_item(LIST(), CONTAINER->edges, subject,
                merge_attributes, NULL); 
        } else if (verb == TLD) {
            CONTAINER->edges = remove_item(LIST(), CONTAINER->edges, subject);
        }
        break;
    default:
        S((state_t)subject->u.l.first->state);
        assert(0); // that should be all
        break;
    }
}
