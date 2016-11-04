/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef PRINT_H
#define PRINT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "thread.h"

uint16_t print_len_frag(FILE * chan, unsigned char *len_frag);
void print_elem(THREAD_t * THREAD, elem_t * elem, int indent, char *sep);
void append_token(THREAD_t * THREAD, char **pos, char tok);
void append_string(THREAD_t * THREAD, char **pos, char *string);
void append_ulong(THREAD_t * THREAD, char **pos, uint64_t integer);
void append_runtime(THREAD_t * THREAD, char **pos, uint64_t run_sec, uint64_t run_ns);

#ifdef __cplusplus
}
#endif

#endif
