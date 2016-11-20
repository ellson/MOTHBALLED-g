/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef COMPARE_H
#define COMPARE_H

#ifdef __cplusplus
extern "C" {
#endif

int compare(elem_t * a, elem_t * b);
int match(elem_t * a, elem_t * b);
void printlist(elem_t * a);

#ifdef __cplusplus
}
#endif

#endif
