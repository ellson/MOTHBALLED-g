/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "thread.h"
#include "reduce.h"

/**
 * merge VALUE for an ATTR which may have seen before
 *                latest VALUE wins
 *
 * @param LIST 
 * @param attr    - attr value 
 * @param key     - key 
 * @return        - key - not used
 */
static elem_t * merge_value(LIST_t *LIST, elem_t *attr, elem_t *key)
{
    THREAD_t *THREAD = (THREAD_t*)LIST;

P(key);
P(attr);
    elem_t *oldattr = key->u.l.first;
    elem_t *oldvalue = oldattr->u.l.next;
    if (oldvalue) {
        free_list(LIST, oldvalue);
        oldattr->u.l.next = NULL;
        key->u.l.last = key->u.l.first;
        key->len = 1;
    }
    elem_t *value = attr->u.l.first->u.l.next;
    if (value) {
        append_transfer(key, value);
    }
    return key;
}

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
#if 0
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
#endif
    return key;
}

void reduce(CONTAINER_t * CONTAINER, elem_t *act, state_t verb)
{
    THREAD_t * THREAD = CONTAINER->THREAD;
    elem_t *subject, *disambig, *attributes, *attr;
    elem_t *newact, *newsubj, *newnouns, *newattrtree = NULL;

    assert(act);
    assert((state_t)act->state == ACT);

    subject = act->u.l.first;
    assert(subject);
    assert((state_t)subject->state == SUBJECT);

//P(act);

    newact = new_list(LIST(), ACT);
    newsubj = new_list(LIST(), SUBJECT);
    newnouns = new_list(LIST(), NOUNS);
    append_transfer(newact, newsubj);
    append_transfer(newsubj, newnouns);
    append_addref(newnouns, subject->u.l.first);
    disambig = subject->u.l.next;
    if (disambig && (state_t)disambig->state == DISAMBIG) {
        elem_t *newdisambig = new_list(LIST(), DISAMBIG);
        append_addref(newdisambig, disambig->u.l.first);
        append_transfer(newsubj, newdisambig);
        attributes = disambig->u.l.next;
    } else {
        attributes = disambig;
        disambig = NULL;
    }
    if (attributes) {
        assert((state_t)attributes->state == ATTRIBUTES);
        attr = attributes->u.l.first;
        while (attr) {
            elem_t *newattr = new_list(LIST(), ATTR);
            append_addref(newattr, attr->u.l.first);
            elem_t *value = attr->u.l.last;
            if (value) {
                append_transfer(newattr, value);
            }
//P(newattr);
            newattrtree = insert_item(LIST(), newattrtree,
                    newattr->u.l.first, merge_value, NULL); 

            attr = attr->u.l.next;
            free_list(LIST(), newattr);
        }
    }
    if (newattrtree) {
        elem_t *newattributes = new_list(LIST(), ATTRIBUTES);
        append_transfer(newattributes, newattrtree);
        append_transfer(newact, newattributes);
    }

P(newact);

    switch ((state_t)subject->u.l.first->state) {
    case NODE:
        if (!verb) {
            CONTAINER->nodes = insert_item(LIST(), CONTAINER->nodes, newact,
                merge_attributes, NULL); 
        } else if (verb == TLD) {
            CONTAINER->nodes = remove_item(LIST(), CONTAINER->nodes, newact);
        }
        break;
    case EDGE:
        if (!verb) {
            CONTAINER->edges = insert_item(LIST(), CONTAINER->edges, newact,
                merge_attributes, NULL); 
        } else if (verb == TLD) {
            CONTAINER->edges = remove_item(LIST(), CONTAINER->edges, newact);
        }
        break;
    default:
        S((state_t)subject->u.l.first->state);
        assert(0); // that should be all
        break;
    }
    free_list(LIST(), newact);
}
