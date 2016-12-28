/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "thread.h"
#include "reduce.h"

/**
 * merge VALUE for an ATTR which already exists
 *                latest VALUE wins
 *
 * @param LIST 
 * @param attr    - attr newvalue 
 * @param key     - key oldvalue
 * @return        - key - not used
 */
static elem_t * merge_value(LIST_t *LIST, elem_t *attr, elem_t *key)
{
    if (key->u.l.next) {
        free_list(LIST, key->u.l.next);
    }
    if ((key->u.l.next = attr->u.l.next)) {
        key->u.l.next->refs++;
    }
    return key;
}

/**
 * recurse over elems of b, merging them into a
 *
 * @param a - tree to be updated with elems from b
 * @param b - tree of elems to be merged into a
 */
static void merge_tree(LIST_t *LIST, elem_t **a, elem_t *b)
{
    if (b->u.t.left) {
        merge_tree(LIST, a, b->u.t.left);
    }
    *a = insert_item(LIST, *a, b->u.t.key, merge_value, NULL);
    if (b->u.t.right) {
        merge_tree(LIST, a, b->u.t.right);
    }
}

/**
 * merge ATTRIBUTEs for a NODE or EDGE which may have existing ATTRIBUTES
 *
 * @param LIST 
 * @param subject - the new subject
 * @param key - the key with previous act
 * @return    - the key with previous act  - not used
 */
static elem_t * merge_attributes(LIST_t *LIST, elem_t *subject, elem_t *key)
{
    elem_t *elem;

    assert((state_t)subject->state == SUBJECT);
    elem = subject->u.l.next;  // disambig or attributes
    if (elem) {
        if ((state_t)elem->state == DISAMBIG) {
            elem = elem->u.l.next;  // attributes
        }
    } 
    if (elem) {
        assert((state_t)elem->state == ATTRIBUTES);
        elem_t *attrtree = elem->u.l.first;
        assert((elemtype_t)attrtree->type == TREEELEM);  // attrs to be merged
        assert((state_t)key->state == SUBJECT);
        elem = key->u.l.next;  // disambig or attributes
        if (elem) {
            if ((state_t)elem->state == DISAMBIG) {
                elem = elem->u.l.next;  // attributes
            }
        } 
        if (elem) {
            assert((elemtype_t)elem->u.l.first->type == TREEELEM);  // prev attrs
            merge_tree(LIST, &(elem->u.l.first), attrtree);
        } else {
//            append_transfer(elem, attrtree);
            // FIXME - old had no attributes: newattrtree = attrtree
            assert(0);
        }
    }
    return key;
}

void reduce(CONTAINER_t * CONTAINER, elem_t *act, state_t verb)
{
    THREAD_t * THREAD = CONTAINER->THREAD;   // for LIST() and P(x)
    elem_t *subject, *disambig, *attributes, *attr;
    elem_t *newact, *newsubj, *newnouns, *newattrtree = NULL;

    assert(act);
    assert((state_t)act->state == ACT);

    subject = act->u.l.first;
    assert(subject);
    assert((state_t)subject->state == SUBJECT);

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
            // insert attr into tree, 
            //     if attr already exists then last one wins
            newattrtree = insert_item(LIST(), newattrtree,
                    attr->u.l.first, merge_value, NULL); 
            attr = attr->u.l.next;
        }
    }
    if (newattrtree) {
        // FIXME - I suspect we don't need all these ...
        elem_t *newattributes = new_list(LIST(), ATTRIBUTES);
        append_transfer(newattributes, newattrtree);
        append_transfer(newact, newattributes);
    }
    switch ((state_t)subject->u.l.first->state) {
    case NODE:
        if (!verb) {
            CONTAINER->nodes = insert_item(LIST(), CONTAINER->nodes,
                    newact->u.l.first, merge_attributes, NULL); 
        } else if (verb == TLD) {
            CONTAINER->nodes = remove_item(LIST(), CONTAINER->nodes,
                    newact->u.l.first);
        }
        break;
    case EDGE:
        if (!verb) {
            CONTAINER->edges = insert_item(LIST(), CONTAINER->edges,
                    newact->u.l.first, merge_attributes, NULL); 
        } else if (verb == TLD) {
            CONTAINER->edges = remove_item(LIST(), CONTAINER->edges,
                    newact->u.l.first);
        }
        break;
    default:
        S((state_t)subject->u.l.first->state);
        assert(0); // that should be all
        break;
    }
    free_list(LIST(), newact);
}
