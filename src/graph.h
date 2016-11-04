/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef GRAPH_H
#define GRAPH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "doact.h"

struct graph_s {               // GRAPH context
    THREAD_t *THREAD;          // THREAD context

// FIXME - move to CONTAINER
    int containment;           // depth of containment
    long stat_containercount;
//

    // FIXME  - do I need to take verb default from parent graph ?
    state_t verb;              // after parsing, 0 "add", TLD "del", QRY "query"

    char has_cousin;           // flag set if a COUSIN is found in any EDGE of the ACT
                               //  (forward EDGE to ancestors for processing)

    // ok as per-graph counts - may also need at THREAD level for total i/o stats
    long stat_inactcount;
    long stat_outactcount;
    long stat_sameas;
    long stat_patternactcount;
    long stat_nonpatternactcount;
    long stat_patternmatches;
};

// functions
success_t graph(GRAPH_t * GRAPH, elem_t * root, state_t si, unsigned char prop, int nest, int repc);

// macro to print an elem
#define P(L) { \
   fprintf(stdout, "\nelem at: %s:%d\n", __FILE__, __LINE__); \
   GRAPH->sep = ' '; \
   print_elem(stdout, L, 0, &(GRAPH->sep)); \
   putc('\n', stdout);}

// macro to print current element count
   #define E() { \
   printf("elemnow at %s:%d is %ld\n", __FILE__, __LINE__, ((LIST_t*)GRAPH)->stat_elemnow);}

// macro to print a stat_t in is text form.
   #define S(state) { \
   fprintf(stdout, "state at: %s:%d is: ", __FILE__, __LINE__); \
   print_len_frag(stdout, NAMEP(state)); \
   putc('\n', stdout);}

#ifdef __cplusplus
}
#endif

#endif
