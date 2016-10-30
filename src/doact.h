/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef DOACT_H
#define DOACT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "pattern.h"
#include "dispatch.h"
#include "reduce.h"
#include "sameas.h"

success_t doact(CONTENT_t *CONTENT, elem_t *act);

#ifdef __cplusplus
}
#endif

#endif
