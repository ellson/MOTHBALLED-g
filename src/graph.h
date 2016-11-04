/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef GRAPH_H
#define GRAPH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "token.h"
#include "ikea.h"

typedef struct {               // GRAPH context
    TOKEN_t TOKEN;             // TOKEN context.  Must be first to allow casting from GRAPH
    SESSION_t *SESSION;        // SESSION context at the top level of nested containment

    state_t verb;              // after parsing, 0 "add", TLD "del", QRY "query"
    char has_cousin;           // flag set if a COUSIN is found in any EDGE of the ACT
                               //  (forward EDGE to ancestors for processing)

    char sep;                  // the next separator
                               // (either 0, or ' ' if following a STRING that
                               // requires a separator,  but may be ignored if
                               // the next character is a token which
                               // implicitly separates.)

    int containment;           // depth of containment
    long stat_inactcount;
    long stat_outactcount;
    long stat_sameas;
    long stat_patternactcount;
    long stat_nonpatternactcount;
    long stat_patternmatches;
    long stat_containercount;


//FIXME -- output context

    FILE *out;                 // typically stdout for parser debug outputs
    style_t style;             // spacing style in emitted outputs
    ikea_store_t *ikea_store;  // persistency
    ikea_box_t *namehash_buckets[64];
} GRAPH_t;

// functions
success_t graph(GRAPH_t * GRAPH, elem_t * root, state_t si, unsigned char prop, int nest, int repc);

// macro to print an elem
#define P(L) { \
   fprintf(GRAPH->out, "\nelem at: %s:%d\n", __FILE__, __LINE__); \
   GRAPH->sep = ' '; \
   print_elem(GRAPH->out, L, 0, &(GRAPH->sep)); \
   putc('\n', GRAPH->out);}

// macro to print current element count
   #define E() { \
   printf("elemnow at %s:%d is %ld\n", __FILE__, __LINE__, ((LIST_t*)GRAPH)->stat_elemnow);}

// macro to print a stat_t in is text form.
   #define S(state) { \
   fprintf(GRAPH->out, "state at: %s:%d is: ", __FILE__, __LINE__); \
   print_len_frag(GRAPH->out, NAMEP(state)); \
   putc('\n', GRAPH->out);}

#ifdef __cplusplus
}
#endif

#endif
