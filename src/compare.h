/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef COMPARE_H
#define COMPARE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "frag.h"

int je_compare(elem_t * a, elem_t * b);
elem_t * je_merge(elem_t * new, elem_t * old);

#ifdef __cplusplus
}
#endif

#endif
