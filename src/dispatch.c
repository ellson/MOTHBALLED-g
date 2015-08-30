#include "libje_private.h"

// Processes an ACT after sameas and pattern substitutions.
//
// - expand OBJECT_LIST
//    - expand ENDPOINT_SETs
//       - promote ENDPOINTS to common ancestor
//           - extract NODES from EDGES for node induction
//               - dispatch NODES (VERB NODE ATTRIBUTES CONTENT)
//                   - if add or delete and CONTENT is from pattern, copy content before modifying (COW)
//           - dispatch EDGE  (VERB EDGE ATTRIBUTES CONTENT)
//               - if add or delete and CONTENT is from pattern, copy content before modifying (COW)
//
// VERBs (add), delete, query

// Edges are owned (stored, rendered) by the common ancestor
// of all the endpoints.
//
// So an edge to an external node:, e.g
//                      <sibling ^/uncle/cousin>
// is dealt with as follows:    // FIXME - not right
//       1. node "sibling" is induced (created in the local
//              graph of siblings, if it isn't already there.)
//       2. the edge is sent though a special channel to
//              the nameless parent of the the current siblings.
//              That parent, prepends its name to all noderefs
//              without a ^/, and removes one ^/ from all the others,
//              So. in the parent's graph, the edge becomes:
//                      <mum/sibling uncle/cousin>
//              If there still ^/ left, then the process
//              is repeated, sending it though the nameless channel to
//              their parent, which applies the same rule:


// this function expands ENDPOINTSETs
void je_expand_r(context_t *C, elem_t *epset, elem_t *leg, elem_t *edges)
{
    elem_t newedge = { 0 };
    elem_t *epsetlast, *ep, *new;

    if (leg) {
        ep = leg->u.list.first;
        while(ep) {

            // keep track of last
            epsetlast = epset->u.list.last;

            // append the next ep for this leg
            new = ref_list(C, ep);    // FIXME - this is a temp list - can we do this without refcounting? Would it save much?
            append_list(epset, new);
            
            // recursively process the rest of the legs
            je_expand_r(C, epset, leg->next, edges);

            // remove this leg's new ep from epset
            epset->u.list.last = epsetlast;
            free_list(C, new);        // FIXME - this is a temp list - can we do this without refcounting?

            // and iterate to next ep for this leg
            ep = ep->next;
        }
    }
    else {
        // if no more legs, then we can create a new edge with the current epset
        // FIXME - need to create ACT with VERB, SUBJECT EDGE, and ATTRIBUTES.
        newedge.state = EDGE;
        ep = epset->u.list.first;
        while (ep) {
            new = ref_list(C, ep);
            append_list(&newedge, new);
            ep = ep->next;
        }
        // and append the new simplified edge to the result
        new = move_list(C, &newedge);
        append_list(edges, new);
    }
}

// this function expands OBJECT_LISTS, and through use of je_expand_r(), ENDPOINTSETs
int je_dispatch_r(context_t * C, elem_t * list, elem_t * attributes, elem_t * nodes, elem_t * edges)
{
    elem_t *elem, *new, *leg, *epset, *ep;
    elem_t newlist = { 0 };
    elem_t newepset = { 0 };
    state_t si;
    int more = 0;

    assert(list->type == (char)LISTELEM);

    elem = list->u.list.first;
    while (elem) {
        si = (state_t) elem->state;
        switch (si) {
        case ATTRIBUTES:
            new = ref_list(C, elem);
            append_list(attributes, new);
            break;
        case SUBJECT:
        case OBJECT:
        case OBJECT_LIST:
            je_dispatch_r(C, elem, attributes, nodes, edges);
            break;
        case NODE:
            new = ref_list(C, elem);
            append_list(nodes, new);
            break;
        case EDGE:
            // build a leg list with endpointsets for each leg
            for (leg = elem->u.list.first; leg; leg = leg->next) {
                epset = leg->u.list.first;
                new = ref_list(C, epset);
                if ((state_t)epset->state == ENDPOINT) { // put singletons into lists too
                    newepset.state = ENDPOINTSET;
                    append_list(&newepset, new);
                    new = move_list(C, &newepset);
                }
                append_list(&newlist, new);

                // induce all sibling nodes....
                assert((state_t)new->state == ENDPOINTSET);

                ep = new->u.list.first;
                while(ep) {
                    assert((state_t)ep->state == ENDPOINT);                    
                    if (ep->u.list.first->state == SIBLING) {
                        new = ref_list(C, ep->u.list.first);
                        append_list(nodes, new);
                        // FIXME - induce CHILDren in this node's container
                    }
                    else if (ep->u.list.first->state == COUSIN) {
                        // FIXME - route to ancestors
                    }
                    else {
                        assert(0);  // shouldn't happen
                    }
#if 0
                    C->sep = ' ';
                    print_list(stdout, ep, 0, &(C->sep));
                    putc('\n', stdout);
#endif
                    ep = ep->next;
                }
            }
            // now recursively generate all combinations of ENDPOINTS in LEGS, and append new simplified EDGEs to result
            je_expand_r(C, &newepset, newlist.u.list.first, edges);
            free_list(C, &newlist);
            break;
        default:
            break;
        }
        elem = elem->next;
    }
    return more;
}

void je_dispatch(container_context_t * CC, elem_t * list)
{
    context_t *C = CC->context;
//    elem_t *elem;
    elem_t attributes = { 0 };
    elem_t nodes = { 0 };
    elem_t edges = { 0 };
    
    assert(list);
    assert(list->type == (char)LISTELEM);

    je_dispatch_r(C, list, &attributes, &nodes, &edges);

#if 0
    switch(C->verb) {
    case QRY:
        putc('?', stdout);
        break;
    case TLD:
        putc('~', stdout);
        break;
    default:
        putc(' ', stdout);
        break;
    }
#endif

#if 1
    C->sep = ' ';
    print_list(stdout, &nodes, 0, &(C->sep));
    putc('\n', stdout);
#endif

#if 1
    C->sep = ' ';
    print_list(stdout, &edges, 0, &(C->sep));
    putc('\n', stdout);
#endif

#if 1
    C->sep = ' ';
    print_list(stdout, &attributes, 0, &(C->sep));
    putc('\n', stdout);
#endif
    free_list(C, &attributes);
    free_list(C, &nodes);
    free_list(C, &edges);
}
