/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef DISPATCH_H
#define DISPATCH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "expand.h"
#include "context.h"

elem_t * dispatch(CONTAINER_t * CONTAINER, elem_t *act, state_t verb);

#ifdef __cplusplus
}
#endif

#endif
