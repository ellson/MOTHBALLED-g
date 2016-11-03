/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef EXPAND_H
#define EXPAND_H

#ifdef __cplusplus
extern "C" {
#endif

#include "context.h"
#include "content.h"

void expand(LIST_t * LIST, elem_t *elem, elem_t *nodes, elem_t *edges);

#ifdef __cplusplus
}
#endif

#endif
