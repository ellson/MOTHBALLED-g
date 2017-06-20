/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef ITER_H
#define ITER_H

#ifdef __cplusplus
extern "C" {
#endif

#define MAXNEST 20

typedef size_t (*writer_fn_t)(const void *ptr, size_t size);

typedef struct {
    elem_t *lnx;
    char *psp;
} lnx_t;

typedef struct {
    elem_t *tnx;
    int dir;
} tnx_t;

typedef struct {
    writer_fn_t writer_fn;
    lnx_t lnxstack[MAXNEST];
    tnx_t tnxstack[MAXNEST];
    uint16_t lsp;
    uint16_t tsp;
    uint16_t len;
    unsigned char *cp, intree;
} iter_t;

void inititer(iter_t *iter, elem_t *elem, writer_fn_t writer_fn);
void inititer0(iter_t *iter, elem_t *elem, writer_fn_t writer_fn);
void nextiter(iter_t *iter);
void skipiter(iter_t *iter);

#ifdef __cplusplus
}
#endif

#endif
