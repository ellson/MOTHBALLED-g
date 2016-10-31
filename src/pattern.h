/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef PATTERN_H
#define PATTERN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "emit.h"
#include "match.h"
#include "tree.h"

void pattern_update(CONTENT_t * CONTENT, elem_t * subject, state_t verb);
elem_t * pattern_match(CONTENT_t * CONTENT, elem_t * subject);

#ifdef __cplusplus
}
#endif

#endif
