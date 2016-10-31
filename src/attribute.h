/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "parse.h"
#include "match.h"
#include "tree.h"

void attribute_update(CONTENT_t * CONTENT, elem_t * subject, state_t verb);

#ifdef __cplusplus
}
#endif

#endif
