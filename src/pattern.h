/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef PATTERN_H
#define PATTERN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "emit.h"
#include "match.h"

void je_pattern(CONTENT_t * CONTENT, elem_t * root, elem_t * subject);

#ifdef __cplusplus
}
#endif

#endif
