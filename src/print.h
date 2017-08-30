/* vim:set shiftwidth=4 ts=8 expandtab: */

/*************************************************************************
 * Copyright (c) 2017 AT&T Intellectual Property
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: John Ellson <john.ellson@gmail.com>
 *************************************************************************/

#ifndef PRINT_H
#define PRINT_H

#ifdef __cplusplus
extern "C" {
#endif

uint16_t print_len_frag(FILE * chan, unsigned char *len_frag);
void print_elem(THREAD_t * THREAD, elem_t * elem, int indent);
void append_token(THREAD_t * THREAD, char **pos, char tok);
void append_string(THREAD_t * THREAD, char **pos, char *string);
void append_ulong(THREAD_t * THREAD, char **pos, uint64_t integer);
void append_runtime(THREAD_t * THREAD, char **pos, uint64_t run_sec, uint64_t run_ns);

// macro to print an elem
#define P(L) { \
   fprintf(((TOKEN_t*)THREAD)->out, "\nelem at: %s:%d\n", __FILE__, __LINE__); \
   THREAD->sep = ' '; \
   print_elem(THREAD, L, 0); \
   putc('\n', ((TOKEN_t*)THREAD)->out);}

// macro to print current element count
   #define E() { \
   fprintf(((TOKEN_t*)THREAD)->out, "elemnow at %s:%d is %ld and fragnow is %ld\n", __FILE__, __LINE__, ((LIST_t*)THREAD)->stat_elemnow, ((LIST_t*)THREAD)->stat_fragnow);}

// macro to print a stat_t in is text form.
   #define S(state) { \
   fprintf(((TOKEN_t*)THREAD)->out, "state at: %s:%d is: ", __FILE__, __LINE__); \
   print_len_frag(((TOKEN_t*)THREAD)->out, NAMEP(state)); \
   putc('\n', ((TOKEN_t*)THREAD)->out);}

#ifdef __cplusplus
}
#endif

#endif
