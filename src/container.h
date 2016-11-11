/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef CONTAINER_H
#define CONTAINER_H

#ifdef __cplusplus
extern "C" {
#endif

struct container_s {            // CONTAINER context
    THREAD_t *THREAD;           // THREAD context

    state_t verb;               // verb for the current ACT
    state_t pattern;            // flag set if '*' occurred in SUBJECT
    state_t sameas;             // flag set if '=' occurred in SUBJECT
    state_t mum;                // flag set if '^' occurred in SUBJECT

    elem_t *previous_subject;   // Set and used by sameas()

    elem_t *patterns;           // complete ACTs where the SUBJECT contains an "*"
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
