/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef DISPATCH_H
#define DISPATCH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "expand.h"
#include "context.h"

void dispatch(CONTENT_t * CONTENT, elem_t **proot);

#ifdef __cplusplus
}
#endif

#endif
