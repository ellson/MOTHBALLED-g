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

#define P(L) {C->sep = ' ';print_list(stdout, L, 0, &(C->sep));putc('\n', stdout);}

#ifdef __cplusplus
}
#endif

#endif
