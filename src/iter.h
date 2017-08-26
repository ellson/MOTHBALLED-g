/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef ITER_H
#define ITER_H

#ifdef __cplusplus
extern "C" {
#endif

#define MAXNEST 20

typedef struct {
    elem_t *lnx;
    char *sep;
} lnx_t;

typedef struct {
    elem_t *tnx;
    int dir;
} tnx_t;

typedef struct {
    lnx_t lstack[MAXNEST];
    tnx_t tstack[MAXNEST];
    uint16_t lsp;
    uint16_t tsp;
    uint16_t len;
    unsigned char *cp, intree;
} iter_t;

int compare(elem_t * a, elem_t * b);
int match(elem_t * a, elem_t * b);
void printt(THREAD_t * THREAD, elem_t * p);

#ifdef __cplusplus
}
#endif

#endif
