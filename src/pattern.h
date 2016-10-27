/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef PATTERN_H
#define PATTERN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "list.h"
#include "context.h"
#include "emit.h"

void je_pattern(container_CONTEXT_t * CC, elem_t * root, elem_t * subject);

#ifdef __cplusplus
}
#endif

#endif
