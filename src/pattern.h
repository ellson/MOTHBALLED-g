/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef PATTERN_H
#define PATTERN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "emit.h"
#include "match.h"
#include "tree.h"

elem_t * pattern(CONTENT_t * CONTENT, elem_t * subject, state_t verb);


#ifdef __cplusplus
}
#endif

#endif
