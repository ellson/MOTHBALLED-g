/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

#include "libje_private.h"

static void je_expand_r(CONTEXT_t *C, elem_t *newepset, elem_t *epset, elem_t *disambig, elem_t *edges);

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
    je_expand_r(C, &newepset, newlist.first, disambig, edges);

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
static void je_expand_r(CONTEXT_t *C, elem_t *newepset, elem_t *epset, elem_t *disambig, elem_t *edges)
{
    LIST_t * LIST = (LIST_t *)C;
    elem_t newedge = { 0 };
    elem_t *ep, *new;

    if (epset) {
        ep = epset->first;
        while(ep) {

            // append the next ep for this epset
            new = ref_list(LIST, ep); 
            append_list(newepset, new);
            
            // recursively process the rest of the epsets
            je_expand_r(C, newepset, epset->next, disambig, edges);

            free_list(LIST, new);

            // and iterate to next ep for this epset
            ep = ep->next;
        }
    }
    else {
        // if no more epsets, then we can create a new edge with the current newepset and dismbig
        newedge.state = EDGE;
        ep = newepset->first;
        while (ep) {
            new = ref_list(LIST, ep);
            append_list(&newedge, new);
            ep = ep->next;
        }
        newepset->first = 0;  // reset newepset for next combo
        if (disambig) {
            new = ref_list(LIST, disambig);
            append_list(&newedge, new);
        }
        // and append the new simplified edge to the result
        new = move_list(LIST, &newedge);
        append_list(edges, new);
    }
}