/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

#include "libje_private.h"

#define BINODE_EDGES 1

static void expand_r(CONTEXT_t *C, elem_t *newepset, elem_t *epset, elem_t *disambig, elem_t *nodes, elem_t *edges);
static void expand_hub(CONTEXT_t *C, elem_t *tail, elem_t *head, elem_t *disambig, elem_t *edges);  // two node edge

/**
 * this function expands and dispatches EDGEs
 * 
 * @param C context
 * @param list - tree representing edge(s)
 * @param nodes - resulting nodes
 * @param edges - resulting simple edges
 */
void je_expand(CONTEXT_t *C, elem_t *list, elem_t *nodes, elem_t *edges)
{
    LIST_t * LIST = (LIST_t *)C;
    elem_t *elem, *epset, *ep, *new, *disambig = NULL;
    elem_t newlist = { 0 };
    elem_t newepset = { 0 };

    assert(list);
    elem = list->first;
    while (elem) {
        switch ((state_t)elem->state) {
        case DISAMBIG:
            disambig = move_list(LIST, elem);
            break;
        case LEG:
            // build a leg list with endpointsets for each leg
            epset = elem->first;
            new = ref_list(LIST, epset);
            if ((state_t)epset->state == ENDPOINT) { // put singletons into lists too
                newepset.state = ENDPOINTSET;
                append_list(&newepset, new);
                new = move_list(LIST, &newepset);
            }
            append_list(&newlist, new);
    
            // induce all sibling nodes....
            assert((state_t)new->state == ENDPOINTSET);
            ep = new->first;
            while(ep) {
                assert((state_t)ep->state == ENDPOINT);                    
                switch (ep->first->state) {
                case SIBLING:
                    new = ref_list(LIST, ep->first);
                    append_list(nodes, new);
                    // FIXME - induce CHILDren in this node's container
                    break;
                case COUSIN:
                    // FIXME - route to ancestors
                    break;
                case ENDPOINT:
                    // FIXME  - if we're going to induce nodes or route to ancestor, then
                    //     this has to be broken down further.
                    // this case occurs if '=' matches an epset
                    //     <(a b) c>
                    //     <= d>
                    new = ref_list(LIST, ep->first);
                    append_list(nodes, new);
                    break;
                default:
                    assert(0);  // shouldn't happen  
                    break;
                }
                ep = ep->next;
            }
            break;
        default:
            assert(0);  // shouldn't be here
            break;
        }
        elem = elem->next;
    }

    // now recursively generate all combinations of ENDPOINTS in LEGS, and append new simplified EDGEs to edges
    expand_r(C, &newepset, newlist.first, disambig, nodes, edges);

    if (disambig) {
        free_list(LIST, disambig);
    }
    free_list(LIST, &newlist);
}

/**
 * this function expands ENDPOINTSETs
 * expands edges like:    <(a b) c>`x
 * into:                  <a c>`x <b c>`x
 *
 * @param C context
 * @param newepset
 * @param epset
 * @param edges
 */
static void expand_r(CONTEXT_t *C, elem_t *newepset, elem_t *epset, elem_t *disambig, elem_t *nodes, elem_t *edges)
{
    LIST_t * LIST = (LIST_t *)C;
    elem_t newedge = { 0 };
    elem_t newlegs = { 0 };
    elem_t * nendpoint = NULL;
    elem_t * nnode;
    elem_t * nnoderef;
    elem_t * nnodeid;
    elem_t * nnodestr;
    elem_t * nshortstr;
    elem_t *ep, *eplast, *new;
    uint64_t hubhash;
    unsigned char hubhash_b64[12];

    if (epset) {
        ep = epset->first;
        while(ep) {

            eplast = newepset->last;

            // append the next ep for this epset
            new = ref_list(LIST, ep); 
            append_list(newepset, new);
            
            // recursively process the rest of the epsets
            expand_r(C, newepset, epset->next, disambig, nodes, edges);

            remove_next_from_list(LIST, newepset, eplast);

            // and iterate to next ep for this epset
            ep = ep->next;
        }
    }
    else {
#ifdef BINODE_EDGES

// FIXME - idea:
//    always compute these hub nodes, then store them with
//    the edge, like disambig. so that the renderers can
//    choose to use or not use.

        // if edge has 1 leg, or has >2 legs
        if ((! newepset->first->next) || (newepset->first->next->next)) {
            // create a special node to represent the hub
    
            je_hash_list(&hubhash, newepset);
            je_long_to_base64(hubhash_b64, &hubhash);
    
// FIXME - this is ugly!

            // create a string fragment with the hash string
            nshortstr = new_shortstr(LIST, EDGE, hubhash_b64);
            nnodestr = new_list(LIST, ABC);
            append_list(nnodestr, nshortstr);

            // new nodeid
            nnodeid = new_list(LIST, NODEID);
            append_list(nnodeid, nnodestr);

            // new noderef
            nnoderef = new_list(LIST, NODEREF);
            append_list(nnoderef, nnodeid);

            // new node
            nnode = new_list(LIST, NODE);
            append_list(nnode, nnoderef);

            // add node to list of nodes for this act
            new = ref_list(LIST, nnode);
            append_list(nodes, new);

            //new endpoint
            nendpoint = new_list(LIST, ENDPOINT);
            append_list(nendpoint, nnode);
        }
#endif

        // if no more epsets, then we can create a new edge with the current newepset and dismbig
        newedge.state = EDGE;
        newlegs.state = ENDPOINTSET;
        if (nendpoint) { // if we have a hub at this point, then we are to split into simple 2-node <tail head> edges
//P(nendpoint);
            ep = newepset->first;
            if (ep) {
                expand_hub(C, ep, nendpoint, disambig, edges);  // first leg is the tail
                ep = ep->next;
            }
            while (ep) {
                expand_hub(C, nendpoint, ep, disambig, edges);  // all other legs are head
                ep = ep->next;
            }
            free_list(LIST, nendpoint);
        }
        else {
            ep = newepset->first;
            while (ep) {
                new = ref_list(LIST, ep);
                append_list(&newlegs, new);
                ep = ep->next;
            }
            new = move_list(LIST, &newlegs);
            append_list(&newedge, new);
    
            if (disambig) {
                new = ref_list(LIST, disambig);
                append_list(&newedge, new);
            }
            // and append the new simplified edge to the result
            new = move_list(LIST, &newedge);
// P(new);
            append_list(edges, new);
        }
    }
}

static void expand_hub(CONTEXT_t *C, elem_t *tail, elem_t *head, elem_t *disambig, elem_t *edges)
{
    LIST_t * LIST = (LIST_t *)C;
    elem_t newedge = { 0 };
    elem_t newlegs = { 0 };
    elem_t *new;

    newedge.state = EDGE;
    newlegs.state = ENDPOINTSET;
    new = ref_list(LIST, tail);
    append_list(&newlegs, new);
    new = ref_list(LIST, head);
    append_list(&newlegs, new);
    new = move_list(LIST, &newlegs);
    append_list(&newedge, new);
    if (disambig) {
        new = ref_list(LIST, disambig);
        append_list(&newedge, new);
    }
    new = move_list(LIST, &newedge);
// P(new);
    append_list(edges, new);
}
