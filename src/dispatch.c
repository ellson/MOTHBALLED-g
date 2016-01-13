/* vim:set shiftwidth=4 ts=8 expandtab: */

#include "libje_private.h"
static void je_dispatch_r(context_t * C, elem_t * list, elem_t * attributes, elem_t * nodes, elem_t * edges);
static void je_assemble_act(context_t *C, elem_t *elem, elem_t *attributes, elem_t *list);

// Processes an ACT after sameas and pattern substitutions.
//

// FIXME - description needs updating

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
        if (C->has_cousin) {
            // FIXME - deal with edges that require help from ancestors
            fprintf(stdout,"EDGE has COUSIN\n");
        }
        else {
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
        }
        break;
    default:
        assert(0);  // shouldn't happen
        break;
    }

    free_list(C, &attributes);
    free_list(C, &nodes);
    free_list(C, &edges);
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
            switch ((state_t)object->state) {
            case OBJECT:
                je_dispatch_r(C, object, attributes, nodes, edges);
                break;
            case OBJECT_LIST:
                object = object->u.list.first;
                assert((state_t)object->state == OBJECT);
                while(object) {
                    je_dispatch_r(C, object, attributes, nodes, edges);
                    object = object->next;
                }
                break;
            default:
                assert(0); //should never get here
                break;
            }
            break;
        case NODE:
            new = ref_list(C, elem);
            append_list(nodes, new);
            break;
        case EDGE:
            je_expand_edge(C, elem, nodes, edges);
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

