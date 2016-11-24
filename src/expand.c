/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

#include "thread.h"
#include "expand.h"

static void expand_r(THREAD_t * THREAD, elem_t *newepset, elem_t *epset, elem_t *disambig, elem_t *nodes, elem_t *edges);

static void expand_hub(THREAD_t * THREAD, elem_t *tail, elem_t *head, elem_t *disambig, elem_t *edges);  // two node edge

/**
 * this function expands EDGEs into:
 *  - a list of nodes reference by the edge
 *  - a list of simple edges, each with the same disambiguation as the compound edge
 * 
 * @param CONTAINER context
 * @param list - a simple or compound edge
 * @param nodes - resulting nodes
 * @param edges - resulting simple edges
 */
void expand(CONTAINER_t * CONTAINER, elem_t *list, elem_t *nodes, elem_t *edges)
{
    THREAD_t *THREAD = CONTAINER->THREAD;
    elem_t *elem, *epset, *ep, *new, *disambig = NULL;
    elem_t *newepset, *newleglist, *singletonepset;

//E();
//S((state_t)list->state);

    assert(list);
    assert((state_t)list->state == EDGE);

    newleglist = new_list(LIST(), ENDPOINTSET);
    elem = list->u.l.first;
    while (elem) {
        switch ((state_t)elem->state) {
            case DISAMBIG:
                disambig = elem;
                break;
            case LEG:
                // build a leg list with endpointsets for each leg
                epset = elem->u.l.first;
                new = ref_list(LIST(), epset);
                singletonepset = NULL;
                if ((state_t)epset->state != ENDPOINTSET) { // put singletons into lists too
                    singletonepset = new_list(LIST(), ENDPOINTSET);
                    append_addref(singletonepset, new);
                    new = ref_list(LIST(), singletonepset);
                }
                // induce all sibling nodes, and gather a cleaned up epset for each leg
                epset = new_list(LIST(), ENDPOINTSET);
                ep = new->u.l.first;
                while(ep) {
                    new = ref_list(LIST(), ep);
                    append_addref(epset, new);
                    switch ((state_t)ep->state) {
                        case SIS:
                            new = ref_list(LIST(), ep);
                            append_addref(nodes, new);
                            // FIXME - induce KIDs in this node's container
                            break;
                        case MUM:
                            // FIXME - route to ancestors
                            break;
                        default:
                            S((state_t)ep->state);
                            assert(0);  // should never get here
                            break;
                    }
                    ep = ep->u.l.next;
                }
                new = ref_list(LIST(), epset);
                append_addref(newleglist, new);
                if (singletonepset) {
                    free_list(LIST(), singletonepset);
                }
                free_list(LIST(), epset);
                break;
            default:
                S((state_t)elem->state);
                assert(0);  // should never get here
                break;
        }
        elem = elem->u.l.next;
    }

//P(newleglist);
    // now recursively generate all combinations of ENDPOINTS in LEGS, and append new simplified EDGEs to edges
    newepset = new_list(LIST(), ENDPOINTSET);
    expand_r(THREAD, newepset, newleglist->u.l.first, disambig, nodes, edges);
    free_list(LIST(), newepset);

//P(nodes);
//P(edges);

    if (disambig) {
        free_list(LIST(), disambig);
    }
    free_list(LIST(), newleglist);

//E();
}

/**
 * This function expands ENDPOINTSETs, expanding edges like:
 *
 *     <(a b c) d>`x
 *
 * into:
 *
 *     <a d>`x
 *     <b d>`x
 *     <c d>`x
 *
 * @param THREAD context
 * @param newepset
 * @param epset
 * @param disambig
 * @param nodes - list of nodes
 * @param edges - list of edges
 */
static void
expand_r(THREAD_t * THREAD, elem_t *newepset, elem_t *epset,
        elem_t *disambig, elem_t *nodes, elem_t *edges)
{
    elem_t *ep, *new;

    if (epset) {
        ep = epset->u.l.first;
        while(ep) {

            elem_t * eplast = newepset->u.l.last;

            // append the next ep for this epset
            new = ref_list(LIST(), ep); 
            append_addref(newepset, new);
            
            // recursively process the rest of the epsets
            expand_r(THREAD, newepset, epset->u.l.next, disambig, nodes, edges);

            remove_next_from_list(LIST(), newepset, eplast);

            // and iterate to next ep for this epset
            ep = ep->u.l.next;
        }
    }
    else {
        elem_t * nendpoint = NULL;

//#define BINODE_EDGES 1

#ifdef BINODE_EDGES

// FIXME - idea:
//    always compute these hub nodes, then store them with
//    the edge, like disambig. so that the renderers can
//    choose to use or not use.

        uint64_t hubhash;

        // if edge has 1 leg, or has >2 legs
        if ((! newepset->u.l.first->u.l.next)
                || (newepset->u.l.first->u.l.next->u.l.next)) {
            // create a special node to represent the hub
            char hubhash_b64[12];
            hash_list(&hubhash, newepset);
            long_to_base64(hubhash_b64, &hubhash);
    
// FIXME - this is ugly!

            // create a string fragment with the hash string
            elem_t * nshortstr = new_shortstr(LIST(), EDGE, hubhash_b64);
            elem_t * nnodestr = new_list(LIST(), ABC);
            append_addref(nnodestr, nshortstr);

            // new nodeid
            elem_t * nnodeid = new_list(LIST(), NODEID);
            append_addref(nnodeid, nnodestr);

            // new noderef
            elem_t * nnoderef = new_list(LIST(), NODEREF);
            append_addref(nnoderef, nnodeid);

            // new node
            elem_t * nnode = new_list(LIST(), NODE);
            append_addref(nnode, nnoderef);

            // add node to list of nodes for this act
            new = ref_list(LIST(), nnode);
            append_addref(nodes, new);

            //new endpoint
            elem_t * nendpoint = new_list(LIST(), ENDPOINT);
            append_addref(nendpoint, nnode);
        }
#endif

        // if no more epsets, then we can create a new edge with the current newepset and dismbig
        if (nendpoint) { // if we have a hub at this point, then we are to split into simple 2-node <tail head> edges
            ep = newepset->u.l.first;
            if (ep) {
                expand_hub(THREAD, ep, nendpoint, disambig, edges);  // first leg is the tail
                ep = ep->u.l.next;
            }
            while (ep) {
                expand_hub(THREAD, nendpoint, ep, disambig, edges);  // all other legs are head
                ep = ep->u.l.next;
            }
            free_list(LIST(), nendpoint);
        }
        else {
            elem_t * newedge = new_list(LIST(), EDGE);
            ep = newepset->u.l.first;
            while (ep) {
                elem_t * newleg = new_list(LIST(), LEG);
                new = ref_list(LIST(), ep);
                append_addref(newleg, new);
                append_transfer(newedge, newleg);
                ep = ep->u.l.next;
            }
    
            if (disambig) {
                new = ref_list(LIST(), disambig);
                append_addref(newedge, new);
            }
            // and append the new simplified edge to the result
            append_transfer(edges, newedge);
        }
    }
}

static void expand_hub(THREAD_t * THREAD, elem_t *tail, elem_t *head, elem_t *disambig, elem_t *edges)
{
    elem_t *new;
    elem_t *newedge = new_list(LIST(), EDGE);
    elem_t *newlegs = new_list(LIST(), ENDPOINTSET);

    new = ref_list(LIST(), tail);
    append_addref(newlegs, new);
    new = ref_list(LIST(), head);
    append_addref(newlegs, new);
    append_addref(newedge, newlegs);
    if (disambig) {
        new = ref_list(LIST(), disambig);
        append_addref(newedge, new);
    }
    append_addref(edges, newedge);
}
