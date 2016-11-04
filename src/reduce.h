/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef REDUCE_H
#define REDUCE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "thread.h"
#include "tree.h"

void reduce(CONTAINER_t * CONTAINER, elem_t *list);

#ifdef __cplusplus
}
#endif

#endif
