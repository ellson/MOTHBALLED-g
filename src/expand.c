/* vim:set shiftwidth=4 ts=8 expandtab: */

/*************************************************************************
 * Copyright (c) 2017 AT&T Intellectual Property
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: John Ellson <john.ellson@gmail.com>
 *************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

#include "thread.h"
#include "expand.h"

static void expand_r(THREAD_t * THREAD, elem_t *newepset, elem_t *epset,
        elem_t *nodes, elem_t *edges);

static void expand_hub(THREAD_t * THREAD, elem_t *tail, elem_t *head,
        elem_t *edges);  // two node edge

/**
 * this function expands EDGEs into:
 *  - a list of nodes reference by the edge
 *  - and a list of simple edges 
 * 
 * @param CONTAINER context
 * @param list - a simple or compound edge
 * @param nodes - resulting nodes
 * @param edges - resulting simple edges
 */
void expand(CONTAINER_t * CONTAINER, elem_t *list, elem_t *nodes, elem_t *edges)
{
    THREAD_t *THREAD = CONTAINER->THREAD;
    elem_t *elem, *ep, *np, *refepset, *newepset, *newleglist;

//E();
//S((state_t)list->state);
//P(list);

    assert(list);
    assert((state_t)list->state == EDGE);

    newleglist = new_list(LIST(), ENDPOINTSET);
    elem = list->u.l.first;
    while (elem) {
        elem_t *leg = new_list(LIST(), ENDPOINTSET);
        switch ((state_t)elem->state) {
            case LEG:
                // build a leg list with endpointsets for each leg
                refepset = new_list(LIST(), ENDPOINTSET);
                ep = elem->u.l.first;
                if ((state_t)ep->state == ENDPOINTSET) {
                    append_addref(refepset, ep->u.l.first);
                } else {
                    // put singletons into lists too
                    append_addref(refepset, ep);
                } 
                // induce all sibling nodes, 
                // and resolve parent/child issues
                ep = refepset->u.l.first;
                while(ep) {
                    switch ((state_t)ep->state) {
                        case SIS:
                            // add NODEID (w.o. PORT) to node list
                            np = ref_list(LIST(), ep->u.l.first);
                            np->state = NODE;
                            append_transfer(nodes, np);

                            // add NODEID (w PORT) to leg list
                            np = ref_list(LIST(), ep);
                            np->state = NODE;
                            append_transfer(leg, np);
                            break;
                        case KID:
                            // FIXME - induce KIDs in this node's container
                            fprintf(stdout, "Ahh, cute kid.\n");
                            break;
                        case MUM:
                            // FIXME - route to ancestors
                            fprintf(stdout, "One for you, Mum.\n");
                            free_list(LIST(), refepset);
                            goto doneleg;
                            break;
                        default:
                            S((state_t)ep->state);
                            assert(0);  // should never get here
                            break;
                    }
                    ep = ep->u.l.next;
                }
                free_list(LIST(), refepset);
                break;
            default:
                S((state_t)elem->state);
                assert(0);  // should never get here
                break;
        }
doneleg:
        if (leg->u.l.first) {
            append_addref(newleglist, leg);
        }
        free_list(LIST(), leg);
        elem = elem->u.l.next;
    }


    // recursively generate all combinations of ENDPOINTS in LEGS,
    //    and append new simplified EDGEs to edges
    newepset = new_list(LIST(), ENDPOINTSET);
    expand_r(THREAD, newepset, newleglist->u.l.first, nodes, edges);
    free_list(LIST(), newepset);
    free_list(LIST(), newleglist);

//P(nodes);
//P(edges);
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
 * @param nodes - list of nodes
 * @param edges - list of edges
 */
static void
expand_r(THREAD_t * THREAD, elem_t *newepset, elem_t *epset,
        elem_t *nodes, elem_t *edges)
{
    elem_t *ep, *new;

    if (epset) {
        ep = epset->u.l.first;
        while(ep) {

            elem_t * eplast = newepset->u.l.last;

            // append the next ep for this epset
            new = ref_list(LIST(), ep); 
            append_transfer(newepset, new);
            
            // recursively process the rest of the epsets
            expand_r(THREAD, newepset, epset->u.l.next, nodes, edges);

            remove_next_from_list(LIST(), newepset, eplast);

            // and iterate to next ep for this epset
            ep = ep->u.l.next;
        }
    } else {
        elem_t * hub = NULL;

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
            sslhash_list(&hubhash, newepset);
            long_to_base64(hubhash_b64, &hubhash);
    
// FIXME - this is ugly!

            // create a string fragment with the hash string
            elem_t * nnodestr = new_shortstr(LIST(), EDGE, hubhash_b64);

            // new node
            elem_t * nnode = new_list(LIST(), NODE);
            append_addref(nnode, nnodestr);

            // add node to list of nodes for this act
            new = ref_list(LIST(), nnode);
            append_addref(nodes, new);

            //new endpoint
            elem_t * hub = new_list(LIST(), ENDPOINT);
            append_addref(hub, nnode);
        }
#endif

        // if no more epsets, then we can create a new edge with the
        // current newepset
        if (hub) { // if we have a hub at this point
            // split into simple 2-node <tail head> edges
            ep = newepset->u.l.first;
            if (ep) { // first leg is the tail
                expand_hub(THREAD, ep, hub, edges);
                ep = ep->u.l.next;
            }
            while (ep) { // all other legs are head
                expand_hub(THREAD, hub, ep, edges);
                ep = ep->u.l.next;
            }
            free_list(LIST(), hub);
        }
        else {
            elem_t * newedge = new_list(LIST(), EDGE);
            ep = newepset->u.l.first;
            while (ep) {
                elem_t * newleg = new_list(LIST(), LEG);
                new = ref_list(LIST(), ep);
                append_transfer(newleg, new);
                append_transfer(newedge, newleg);
                ep = ep->u.l.next;
            }
    
            // and append the new simplified edge to the result
            append_transfer(edges, newedge);
        }
    }
}

static void expand_hub(THREAD_t * THREAD, elem_t *tail,
        elem_t *head, elem_t *edges)
{
    elem_t *new;
    elem_t *newedge = new_list(LIST(), EDGE);
    elem_t *newlegs = new_list(LIST(), ENDPOINTSET);

    new = ref_list(LIST(), tail);
    append_addref(newlegs, new);
    new = ref_list(LIST(), head);
    append_addref(newlegs, new);
    append_addref(newedge, newlegs);
    append_addref(edges, newedge);
}
