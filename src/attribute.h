/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "content.h"
#include "match.h"
#include "tree.h"

elem_t * attrid_merge(CONTENT_t * CONTENT, elem_t * attributes);

#ifdef __cplusplus
}
#endif

#endif
