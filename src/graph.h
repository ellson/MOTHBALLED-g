/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef GRAPH_H
#define GRAPH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "doact.h"
#include "sameas.h"

success_t content(CONTAINER_t * CONTAINER, elem_t * root, state_t si, unsigned char prop, int nest, int repc);

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
