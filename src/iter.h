/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef ITER_H
#define ITER_H

#ifdef __cplusplus
extern "C" {
#endif

#define MAXNEST 20

typedef struct {
    elem_t *nextstack[MAXNEST];
    char *spp[MAXNEST];
    unsigned char *cp;
    uint16_t sp;
    uint16_t len;
} iter_t;


void inititer(iter_t *iter, elem_t *elem);
void nextiter(iter_t *iter);
void skipiter(iter_t *iter);

#ifdef __cplusplus
}
#endif

#endif
