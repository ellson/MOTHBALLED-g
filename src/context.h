/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef CONTEXT_H
#define CONTEXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "libje.h"
#include "token.h"
#include "ikea.h"

struct context_s {             // input_context
    TOKEN_t TOKEN;             // Must be first (to allow casting from CONTEXT_t)

    char *progname;            // name of program
    FILE *out;                 // typically stdout for parser debug outputs

    char *username;            // set by first call to g_session
    char *hostname;            // ditto
    char has_cousin;           // flag set if a COUSIN is found in any EDGE of the ACT
                               //  (forward EDGE to ancestors for processing)
    char needstats;            // flag set if -s on command line
    char sep;                  // the next separator
                               // (either 0, or ' ' if following a STRING that
                               // requires a separator,  but may be ignored if
                               // the next character is a token which implicitly separates.)
    style_t style;             // spacing style in emitted outputs
    int containment;           // depth of containment
    ikea_store_t *ikea_store;  // persistency
    ikea_box_t *namehash_buckets[64];
    elem_t *hash_buckets[64];  // 64 buckets of name hashes and FILE*.
    long stat_inactcount;
    long stat_outactcount;
    long stat_sameas;
    long stat_patternactcount;
    long stat_nonpatternactcount;
    long stat_patternmatches;
    long stat_containercount;

#if defined(HAVE_CLOCK_GETTIME)
    // Y2038-unsafe struct - but should be ok for uptime
    // ref: https://sourceware.org/glibc/wiki/Y2038ProofnessDesign
    struct timespec uptime;     // time with subsec resolution since boot, used as the base for runtime calculations
#else
    // Y2038-unsafe struct - but should be ok for uptime
    // ref: https://sourceware.org/glibc/wiki/Y2038ProofnessDesign
    struct timeval uptime;      // time with subsec resolution since boot, used as the base for runtime calculations
#endif
    pid_t pid;
};

typedef struct {    // container_context
    CONTEXT_t *C;               // the input context
    elem_t subject;             // Preceeding ACT's subject, until this ACT's
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
    elem_t node_pattern_acts;   // complete ACTs where the NODE subject contains an "*"
    elem_t edge_pattern_acts;   // complete ACTs where the EDGE subject contains an "*"
    elem_t *nodes;              // tree of unique nodes
    elem_t *edges;              // tree of unique edges

    ikea_box_t *ikea_box;       // box for these contents
    FILE *out;                  // the output file for this container

    // FIXME  - place for fork header for layout process...

} container_CONTEXT_t;

#ifdef __cplusplus
}
#endif

#endif
