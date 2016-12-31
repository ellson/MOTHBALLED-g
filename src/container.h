/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef CONTAINER_H
#define CONTAINER_H

#ifdef __cplusplus
extern "C" {
#endif

struct container_s {      // CONTAINER context
    THREAD_t *THREAD;     // parent THREAD

    state_t verb;         // verb for the current ACT
    state_t subj_has_ast; // flag set if '*' occurred in SUBJECT
    state_t attr_has_ast; // flag set if '*' occurred in ATTRIBUTES
    state_t has_sameas;   // flag set if '=' occurred in SUBJECT
    state_t has_mum;      // flag set if '^' occurred in SUBJECT
    state_t has_node;     // flag set if NODE occurred in SUBJECT
    state_t has_edge;     // flag set if EDHE occurred in SUBJECT

    elem_t *previous;     // previous ACT for sameas
    elem_t *node_patterns;// complete ACTs where the NODE SUBJECT contains an "*"
    elem_t *edge_patterns;// complete ACTs where the EDGE SUBJECT contains an "*"
    elem_t *nodes;        // tree of unique NODEs
    elem_t *edges;        // tree of unique EDGEs

    // stats
    long stat_containercount;
    long stat_inactcount;
    long stat_sameas;
    long stat_patternnodecount;
    long stat_patternedgecount;
    long stat_nonpatternactcount;
    long stat_patternmatches;
    long stat_outactcount;
};

success_t container(THREAD_t *THREAD);

#ifdef __cplusplus
}
#endif

#endif
