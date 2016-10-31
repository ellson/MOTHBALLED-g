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
void print_elem(FILE * chan, elem_t * elem, int indent, char *sep);
void print_tree(FILE *chan, elem_t * p, char *sep);

#ifdef __cplusplus
}
#endif

#endif
