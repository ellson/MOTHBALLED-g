//
// Edged are owned (stored, rendered) by the common ancestor
// of all the endpoints.
//
// So an edge to an external node:, e.g
//                      <sibling ^/uncle/cousin>
// is dealt with as follows:
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

//              FIXME
