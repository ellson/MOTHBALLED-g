/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef CONTAINER_H
#define CONTAINER_H

#ifdef __cplusplus
extern "C" {
#endif

struct container_s {            // CONTAINER context
    GRAPH_t GRAPH;              // GRAPH contents context. Must be first to allow casting from CONTAINER
    elem_t *previous_subject;   // For use by sameas()
    char is_pattern;            // flag set if '*' occurred in SUBJECT
    state_t subject_type;       // set by sameas() to record if the SUBJECT is NODE(s),
                                //   or EDGE(s), and to check that it is not a mix
                                //   of NODE(s) and EDGE(s).
    elem_t *node_pattern_acts;  // complete ACTs where the NODE subject contains an "*"
    elem_t *edge_pattern_acts;  // complete ACTs where the EDGE subject contains an "*"
    elem_t *nodes;              // tree of unique nodes
    elem_t *edges;              // tree of unique edges

// FIXME - move to THREAD
    ikea_box_t *ikea_box;       // box for these contents
//

};

// functions
success_t container(GRAPH_t *GRAPH);

#ifdef __cplusplus
}
#endif

#endif
