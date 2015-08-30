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
static void je_expand_r(context_t *C, elem_t *epset, elem_t *leg, elem_t *edges)
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

// this function expands EDGEs
static void je_expand(context_t *C, elem_t *elem, elem_t *nodes, elem_t *edges)
{
    elem_t *leg, *epset, *ep, *new;
    elem_t newlist = { 0 };
    elem_t newepset = { 0 };

    assert(elem);

    // build a leg list with endpointsets for each leg
    leg = elem->u.list.first;
    assert((state_t)leg->state == LEG);
    while (leg) {
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
            switch (ep->u.list.first->state) {
            case SIBLING:
                new = ref_list(C, ep->u.list.first);
                append_list(nodes, new);
                // FIXME - induce CHILDren in this node's container
                break;
            case COUSIN:
                // FIXME - route to ancestors
                break;
            default:
                assert(0);  // shouldn't happen
                break;
            }
            ep = ep->next;
        }
        leg = leg->next;
    }
    // now recursively generate all combinations of ENDPOINTS in LEGS, and append new simplified EDGEs to edges
    je_expand_r(C, &newepset, newlist.u.list.first, edges);
    free_list(C, &newlist);
}

// this function expands OBJECT_LISTS of NODES or EDGES, and then expands ENPOINTSETS in EDGES
static void je_dispatch_r(context_t * C, elem_t * list, elem_t * attributes, elem_t * nodes, elem_t * edges)
{
    elem_t *elem, *new, *object;
    state_t si;

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
            object = elem->u.list.first;
            if ((state_t)object->state == OBJECT) {
                je_dispatch_r(C, object, attributes, nodes, edges);
            }
            else if ((state_t)object->state == OBJECT_LIST) {
                object = object->u.list.first;
                assert((state_t)object->state == OBJECT);
                while(object) {
                    je_dispatch_r(C, object, attributes, nodes, edges);
                    object = object->next;
                }
            }
            else {
                assert(0); //should never get here
            }
            break;
        case NODE:
            new = ref_list(C, elem);
            append_list(nodes, new);
            break;
        case EDGE:
            je_expand(C, elem, nodes, edges);
            break;
        default:
            break;
        }
        elem = elem->next;
    }
}

static void je_assemble_act(context_t *C, elem_t *elem, elem_t *attributes, elem_t *list)
{
    elem_t act = { 0 };
    elem_t verb = { 0 };
    elem_t *new;

    act.state = ACT;

    switch(C->verb) {
    case QRY:
    case TLD:
        verb.state = C->verb;
        new = move_list(C, &verb);
        append_list(&act, new);
        break;
    default:
        break;
    }
    new = ref_list(C, elem);
    append_list(&act, new);
    if (attributes && attributes->u.list.first) {
        new = ref_list(C, attributes->u.list.first);
        append_list(&act, new);
    }

    new = move_list(C, &act);
    append_list(list, new);
}

void je_dispatch(container_context_t * CC, elem_t * list)
{
    context_t *C = CC->context;
    elem_t attributes = { 0 };
    elem_t nodes = { 0 };
    elem_t edges = { 0 };
    elem_t *elem;
    
    assert(list);
    assert(list->type == (char)LISTELEM);

    // expand OBJECT_LIST and ENDPOINTSETS
    je_dispatch_r(C, list, &attributes, &nodes, &edges);

    // if NODE ACT ... for each NODE from nodes, generate new ACT: verb node attributes
    // else if EDGE ACT ... for each NODEREF, generate new ACT: verb node
    //                      for each EDGE, generate new ACT: verb edge attributes

    free_list(C, list);  // free old ACT to be replace by these new expanded ACTS
    switch (CC->subject_type) {
    case NODE:
        elem = nodes.u.list.first;
        while (elem) {
            assert ((state_t)elem->state == NODE);
            je_assemble_act(C,elem,&attributes,list);
            elem = elem->next;
        }
        break;
    case EDGE:
        elem = nodes.u.list.first;
        while (elem) {
            // FIXME - may have nattributes on LEG
            je_assemble_act(C,elem,NULL,list);
            elem = elem->next;
        }
        elem = edges.u.list.first;
        while (elem) {
            je_assemble_act(C,elem,&attributes,list);
            elem = elem->next;
        }
        break;
    default:
        assert(0);  // shouldn't happen
        break;
    }




#if 0
    C->sep = ' ';
    print_list(stdout, &nodes, 0, &(C->sep));
    putc('\n', stdout);
#endif

#if 0
    C->sep = ' ';
    print_list(stdout, &edges, 0, &(C->sep));
    putc('\n', stdout);
#endif

#if 0
    C->sep = ' ';
    print_list(stdout, &attributes, 0, &(C->sep));
    putc('\n', stdout);
#endif

#if 0
    C->sep = ' ';
    print_list(stdout, list, 0, &(C->sep));
    putc('\n', stdout);
#endif

    free_list(C, &attributes);
    free_list(C, &nodes);
    free_list(C, &edges);
}
