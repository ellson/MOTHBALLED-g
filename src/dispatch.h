/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef DISPATCH_H
#define DISPATCH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "list.h"
#include "context.h"
#include "expand.h"

void je_dispatch(CONTENT_t * CC, elem_t * root);

#ifdef __cplusplus
}
#endif

#endif
