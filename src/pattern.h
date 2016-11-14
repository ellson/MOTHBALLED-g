/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef PATTERN_H
#define PATTERN_H

#ifdef __cplusplus
extern "C" {
#endif

void pattern_update(CONTAINER_t * CONTAINER, elem_t * act);
void pattern_remove(CONTAINER_t * CONTAINER, elem_t * act);
void pattern_match(CONTAINER_t * CONTAINER, elem_t * act);

#ifdef __cplusplus
}
#endif

#endif
