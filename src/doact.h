/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef DOACT_H
#define DOACT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "attribute.h"
#include "pattern.h"
#include "dispatch.h"
#include "reduce.h"

success_t doact(CONTAINER_t *CONTAINER, elem_t *act);

#ifdef __cplusplus
}
#endif

#endif
