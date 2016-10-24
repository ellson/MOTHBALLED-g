/* vim:set shiftwidth=4 ts=8 expandtab: */

elem_t * search(elem_t * p, elem_t * key);
void list(elem_t * p, char *sep);
elem_t * insert(LIST_t * LIST, elem_t * p, elem_t * key);
elem_t * remove_item(LIST_t * LIST, elem_t * p, elem_t * key);
