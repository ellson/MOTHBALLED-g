/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef SSLHASH_H
#define SSLHASH_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "frag.h"

#ifdef __cplusplus
extern "C" {
#endif

void je_sslhash_list(uint64_t *hash, elem_t *list);

#ifdef __cplusplus
}
#endif

#endif
