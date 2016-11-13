/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef MERGE_H
#define MERGE_H

#ifdef __cplusplus
extern "C" {
#endif

void merge_key(LIST_t *LIST, elem_t **key, elem_t *oldkey);
void merge_act(LIST_t *LIST, elem_t **key, elem_t *oldkey);

#ifdef __cplusplus
}
#endif

#endif
