/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef MATCH_H
#define MATCH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "context.h"

success_t match(CONTAINER_t * CONTAINER, elem_t * subject, elem_t * pattern);

#ifdef __cplusplus
}
#endif

#endif
