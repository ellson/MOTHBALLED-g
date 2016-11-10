/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef TREE_H
#define TREE_H

#ifdef __cplusplus
extern "C" {
#endif

elem_t * search_item(elem_t * p, elem_t * key);
elem_t * insert_item(LIST_t * LIST, elem_t * p, elem_t **key,
                 void (*merge)(LIST_t* LIST, elem_t **key, elem_t *oldkey) );
elem_t * remove_item(LIST_t * LIST, elem_t * p, elem_t * key);

#ifdef __cplusplus
}
#endif

#endif
