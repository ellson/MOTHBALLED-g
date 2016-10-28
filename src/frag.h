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
void print_list(FILE * chan, elem_t * list, int indent, char *sep);

#define P(C, L) {((PARSE_t*)C)->sep = ' ';print_list(stdout, L, 0, &(((PARSE_t*)C)->sep));putc('\n', stdout);}

#define E(C,loc) {printf("elemnow at %s is %d\n", loc, ((LIST_t*)C)->stat_elemnow);}

#ifdef __cplusplus
}
#endif

#endif
