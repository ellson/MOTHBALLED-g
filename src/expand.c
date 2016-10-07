/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

#include "libje_private.h"

static void je_expand_r(CONTEXT_t *C, elem_t *epset, elem_t *leg, elem_t *edges);

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
    elem_t *elem, *epset, *ep, *new;
    elem_t newlist = { 0 };
    elem_t newepset = { 0 };

    assert(list);

    elem = list->first;
    while (elem) {
        switch ((state_t)elem->state) {
        case DISAMBIG:
            // FIXME - now what?
            //
            // rather than waiting for this case to show up at the end, I think
            // it needs to be extracted first, so that it can be passed into 
            // je_expand_r() where the new edges are created.
            //
            // why are we finding it here ?   maybe this should be extracted in the caller?
            //          EDGE ::= LAN _LEG+ RAN DISAMBIG?
            fprintf(stderr,"FIXME - Found DISAMBIG\n");
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
    je_expand_r(C, &newepset, newlist.first, edges);
    free_list(LIST, &newlist);
}
/**
 * this function expands ENDPOINTSETs
 * expands edges like:    <(a b) c>
 * into:                  <a c><b c>
 *
 * @param C context
 * @param epset
 * @param leg
 * @param edges
 */
static void je_expand_r(CONTEXT_t *C, elem_t *epset, elem_t *leg, elem_t *edges)
{
    LIST_t * LIST = (LIST_t *)C;
    elem_t newedge = { 0 };
    elem_t *ep, *new;

    if (leg) {
        ep = leg->first;
        while(ep) {

            // append the next ep for this leg
            new = ref_list(LIST, ep); 
            append_list(epset, new);
            
            // recursively process the rest of the legs
            je_expand_r(C, epset, leg->next, edges);

            free_list(LIST, new);

            // and iterate to next ep for this leg
            ep = ep->next;
        }
    }
    else {
        // if no more legs, then we can create a new edge with the current epset
        newedge.state = EDGE;
        ep = epset->first;
        while (ep) {
            new = ref_list(LIST, ep);
            append_list(&newedge, new);
            ep = ep->next;
        }
        // and append the new simplified edge to the result
        new = move_list(LIST, &newedge);
        append_list(edges, new);
    }
}
