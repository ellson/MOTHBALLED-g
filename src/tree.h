/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef TREE_H
#define TREE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "context.h"
#include "compare.h"
#include "frag.h"

elem_t * search(elem_t * p, elem_t * key);
void print_tree(PARSE_t * C, elem_t * p);
elem_t * insert(LIST_t * LIST, elem_t * p, elem_t * key);
elem_t * remove_item(LIST_t * LIST, elem_t * p, elem_t * key);

#ifdef __cplusplus
}
#endif

#endif
