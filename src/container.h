/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef CONTAINER_H
#define CONTAINER_H

#ifdef __cplusplus
extern "C" {
#endif

struct container_s {            // CONTAINER context
    THREAD_t *THREAD;           // THREAD context

    state_t is_pattern;         // flag set if '*' occurred in SUBJECT
    state_t has_sameas;         // flag set if '=' occurred in SUBJECT
    state_t mum;                // flag set if '^' occurred in SUBJECT
    state_t verb;               // verb for the current ACT

    elem_t *previous_subject;   // Set and used by sameas()
    state_t subject_type;       // Set by sameas() to record if the SUBJECT is NODE(s),
                                //   or EDGE(s), and to check that it is not a mix
                                //
    elem_t *node_pattern_acts;  // complete ACTs where the NODE subject contains an "*"
    elem_t *edge_pattern_acts;  // complete ACTs where the EDGE subject contains an "*"
    elem_t *nodes;              // tree of unique nodes
    elem_t *edges;              // tree of unique edges

    ikea_box_t *ikea_box;       // box for these contents

    // stats
    long stat_containercount;
    long stat_inactcount;  
    long stat_sameas;
    long stat_patternactcount;
    long stat_nonpatternactcount;
    long stat_patternmatches;
    long stat_outactcount;
};

success_t container(THREAD_t *THREAD);

#ifdef __cplusplus
}
#endif

#endif
