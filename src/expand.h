/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef EXPAND_H
#define EXPAND_H

#ifdef __cplusplus
extern "C" {
#endif

#include "list.h"
#include "context.h"

void je_expand(PARSE_t * PARSE, elem_t *elem, elem_t *nodes, elem_t *edges);

#ifdef __cplusplus
}
#endif

#endif
