/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef PARSE_H
#define PARSE_H

#ifdef __cplusplus
extern "C" {
#endif

// functions
success_t parse(CONTAINER_t * CONTAINER, elem_t * root, state_t si, unsigned char prop, int nest, int repc, state_t bi);

#ifdef __cplusplus
}
#endif

#endif
