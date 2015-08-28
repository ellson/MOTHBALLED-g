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


void je_dispatch(container_context_t * CC, elem_t * root)
{
    context_t *C = CC->context;

    C->sep = ' ';

    switch(CC->context->verb) {
    case QRY:
        fprintf(stdout,"\nquery  ");
        break;
    case TLD:
        fprintf(stdout,"\ndelete ");
        break;
    default:
        fprintf(stdout,"\nadd    ");
        break;
    }
    print_list(stdout, root, 7, &(C->sep));
}
