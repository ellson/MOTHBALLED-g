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


void je_dispatch_r(container_context_t * CC, elem_t * list, elem_t * attributes, elem_t * nodes, elem_t * edges)
{
    context_t *C = CC->context;
    elem_t *elem, *new;
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
        case OBJECT:
        case OBJECT_LIST:
            je_dispatch_r(CC, elem, attributes, nodes, edges);
            break;
        case NODE:
            new = ref_list(C, elem);
            append_list(nodes, new);
            break;
        case EDGE:
            new = ref_list(C, elem);
            append_list(edges, new);
            break;
        default:
            break;
        }
        elem = elem->next;
    }
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

    je_dispatch_r(CC, list, &attributes, &nodes, &edges);

#if 0
    switch(CC->context->verb) {
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

    C->sep = ' ';
    print_list(stdout, root, 1, &(C->sep));
    putc('\n', stdout);
#endif

#if 0
    C->sep = ' ';
    print_list(stdout, &attributes, 1, &(C->sep));
    putc('\n', stdout);
#endif

#if 0
    C->sep = ' ';
    print_list(stdout, &nodes, 1, &(C->sep));
    putc('\n', stdout);
#endif

#if 0
    C->sep = ' ';
    print_list(stdout, &edges, 1, &(C->sep));
    putc('\n', stdout);
#endif

    free_list(C, &attributes);
    free_list(C, &nodes);
    free_list(C, &edges);
}
