/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef FRAG_H
#define FRAG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "grammar.h"
#include "list.h"

uint16_t print_len_frag(FILE * chan, unsigned char *len_frag);
void print_frags(FILE * chan, state_t state, elem_t * elem, char *sep);
void print_elem(LIST_t * LIST, elem_t * elem, int indent, char *sep);

// macro to print an elem
#define P(C, L) { \
    printf("\nelem at: %s:%d\n", __FILE__, __LINE__); \
    ((PARSE_t*)C)->sep = ' '; \
    print_elem((LIST_t*)C, L, 0, &(((PARSE_t*)C)->sep)); \
    putc('\n', stdout);}

// macro to print current element count
#define E(C) { \
    printf("elemnow at %s:%d is %ld\n", __FILE__, __LINE__, ((LIST_t*)C)->stat_elemnow);}

// macro to print a stat_t in is text form.
#define S(state) { \
    printf("state at: %s:%d is: ", __FILE__, __LINE__); \
    print_len_frag(stdout, NAMEP(state)); \
    putc('\n', stdout);}

#ifdef __cplusplus
}
#endif

#endif
