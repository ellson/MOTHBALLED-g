/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef CONTAINER_H
#define CONTAINER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "graph.h"
#include "ikea.h"

typedef struct {                // CONTAINER context
    GRAPH_t GRAPH;              // GRAPH contents context. Must be first to allow casting from CONTAINER
    elem_t *subject;            // Preceeding ACT's subject, until this ACT's
                                // SUBJECT has been parsed and processd by sameas()
                                //   - at which point it becomes this ACT's subject.
                                // (So: in SUBJECT parsing it is the previous ACT's
                                // subject and used for sameas() substitutions once
                                // a new SUBJECT has been parsed. For ATTRIBUTES
                                // and CONTAINERS it is this ACT.   It is the basis
                                // of the name for the output files for contents.)
    char is_pattern;            // flag set if '*' occurred in SUBJECT
    state_t subject_type;       // set by sameas() to record if the SUBJECT is NODE(s),
                                //   or EDGE(s), and to check that it is not a mix
                                //   of NODE(s) and EDGE(s).
    elem_t *node_pattern_acts;  // complete ACTs where the NODE subject contains an "*"
    elem_t *edge_pattern_acts;  // complete ACTs where the EDGE subject contains an "*"
    elem_t *nodes;              // tree of unique nodes
    elem_t *edges;              // tree of unique edges

    ikea_box_t *ikea_box;       // box for these contents
    FILE *out;                  // the output file for this container

    // FIXME  - place for fork header for layout process...

} CONTAINER_t;

// functions
success_t container(GRAPH_t *GRAPH);

#ifdef __cplusplus
}
#endif

#endif
