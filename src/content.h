/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef CONTENT_H
#define CONTENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "doact.h"
#include "sameas.h"

success_t content(CONTENT_t * CONTENT, elem_t * root, state_t si, unsigned char prop, int nest, int repc);

// macro to print an elem
#define P(L) { \
   fprintf(PARSE->out, "\nelem at: %s:%d\n", __FILE__, __LINE__); \
   PARSE->sep = ' '; \
   print_elem(PARSE->out, L, 0, &(PARSE->sep)); \
   putc('\n', PARSE->out);}

// macro to print current element count
   #define E() { \
   printf("elemnow at %s:%d is %ld\n", __FILE__, __LINE__, ((LIST_t*)PARSE)->stat_elemnow);}

// macro to print a stat_t in is text form.
   #define S(state) { \
   fprintf(PARSE->out, "state at: %s:%d is: ", __FILE__, __LINE__); \
   print_len_frag(PARSE->out, NAMEP(state)); \
   putc('\n', PARSE->out);}

#ifdef __cplusplus
}
#endif

#endif
