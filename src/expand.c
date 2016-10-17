/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

#include "libje_private.h"

#define BINODE_EDGES 1

static void expand_r(CONTEXT_t *C, elem_t *newepset, elem_t *epset, elem_t *disambig, elem_t *hub, elem_t *edges);
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
    elem_t *elem, *epset, *ep, *new, *disambig = NULL, *hub=NULL;
    elem_t newlist = { 0 };
    elem_t newepset = { 0 };
    elem_t newnode = { 0 };
    elem_t newnoderef = { 0 };
    elem_t newnodeid = { 0 };
    elem_t newnodestr = { 0 };
    frag_elem_t newnodefrag = { 0 };

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

#ifdef BINODE_EDGES
    // if edge has 1 leg, or has >2 legs
    if ((! newlist.first->next) || (newlist.first->next->next)) {
        // create a special node to represent the hub

        uint64_t hubhash;
        unsigned char hubhash_b64[12];

        je_hash_list(&hubhash, &newlist);
        je_long_to_base64(hubhash_b64, &hubhash);

        // FIXME - this is ugly! and needs a hack in list.c to not free frag

        newnodefrag.state = ABC;
        newnodefrag.type = FRAGELEM;
        newnodefrag.inbuf = NULL;
        newnodefrag.frag = hubhash_b64;
        newnodefrag.len = 11;

        newnodestr.first = &newnodefrag;
        newnodestr.state = ABC;
        new = move_list(LIST, &newnodestr);
        append_list(&newnodeid, new);

        newnodeid.state = NODEID;
        new = move_list(LIST, &newnodeid);
        append_list(&newnoderef, new);

        newnoderef.state = NODEREF;
        new = move_list(LIST, &newnoderef);
        append_list(&newnode, new);

        newnode.state = NODE;
        hub = move_list(LIST, &newnode);
        new = ref_list(LIST, hub);
        append_list(nodes, new);
    }
#endif

    // now recursively generate all combinations of ENDPOINTS in LEGS, and append new simplified EDGEs to edges
    expand_r(C, &newepset, newlist.first, disambig, hub, edges);

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
static void expand_r(CONTEXT_t *C, elem_t *newepset, elem_t *epset, elem_t *disambig, elem_t *hub, elem_t *edges)
{
    LIST_t * LIST = (LIST_t *)C;
    elem_t newedge = { 0 };
    elem_t newlegs = { 0 };
    elem_t newep = { 0 };
    elem_t *ep, *new;

    if (epset) {
        ep = epset->first;
        while(ep) {

            // append the next ep for this epset
            new = ref_list(LIST, ep); 
            append_list(newepset, new);
            
            // recursively process the rest of the epsets
            expand_r(C, newepset, epset->next, disambig, hub, edges);

            free_list(LIST, new);

            // and iterate to next ep for this epset
            ep = ep->next;
        }
    }
    else {
        // if no more epsets, then we can create a new edge with the current newepset and dismbig
        newedge.state = EDGE;
        newlegs.state = ENDPOINTSET;
        if (hub) { // if we have a hub at this point, then we are to split into simple 2-node <tail head> edges
            newep.state = ENDPOINT;
            new = ref_list(LIST, hub);
            append_list(&newep, new);
            ep = newepset->first;
            if (ep) {
                expand_hub(C, ep, &newep, disambig, edges);  // first leg is the tail
                ep = ep->next;
            }
            while (ep) {
                expand_hub(C, &newep, ep, disambig, edges);  // all other legs are head
                ep = ep->next;
            }
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
        // FIXME - this doesn't look right, and may be an elem leak
        newepset->first = 0;  // reset newepset for next combo
    
    }
}

static void expand_hub(CONTEXT_t *C, elem_t *tail, elem_t *head, elem_t *disambig, elem_t *edges)
{
    LIST_t * LIST = (LIST_t *)C;
    elem_t newedge = { 0 };
    elem_t newlegs = { 0 };
    elem_t *ep, *new;

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
