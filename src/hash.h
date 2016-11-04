/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef HASH_H
#define HASH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "success.h"
#include "list.h"

#if 0
hash_elem_t * hash_bucket(GRAPH_t * GRAPH, uint64_t hash);
#endif
void hash_list(uint64_t *hash, elem_t *list);
void long_to_base64(char *b64string, const uint64_t *hash);
success_t base64_to_long(const char *b64string, uint64_t *hash);

#ifdef __cplusplus
}
#endif

#endif
