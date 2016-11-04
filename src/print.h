/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef PRINT_H
#define PRINT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "thread.h"

uint16_t print_len_frag(FILE * chan, unsigned char *len_frag);
void print_frags(FILE * chan, state_t state, elem_t * elem, char *sep);
void print_elem(FILE * chan, elem_t * elem, int indent, char *sep);
void print_tree(FILE *chan, elem_t * p, char *sep);
void append_token(GRAPH_t * GRAPH, char **pos, char tok);
void append_string(GRAPH_t * GRAPH, char **pos, char *string);
void append_ulong(GRAPH_t * GRAPH, char **pos, uint64_t integer);
void append_runtime(GRAPH_t * GRAPH, char **pos, uint64_t run_sec, uint64_t run_ns);

#ifdef __cplusplus
}
#endif

#endif
