/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef MERGE_H
#define MERGE_H

#ifdef __cplusplus
extern "C" {
#endif

elem_t * merge_attrid(LIST_t *LIST, elem_t *attrid, elem_t *key);
elem_t * merge_attributes(LIST_t *LIST, elem_t *attributes, elem_t *key);
elem_t * merge_pattern(LIST_t *LIST, elem_t *act, elem_t *key);

#ifdef __cplusplus
}
#endif

#endif
