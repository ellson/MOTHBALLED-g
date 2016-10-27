/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef EMIT_H
#define EMIT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "context.h"

typedef struct {
    char *name;
    void (*initialize) (CONTEXT_t * C);
    void (*finalize) (CONTEXT_t * C);

    void (*start_file) (CONTEXT_t * C);
    void (*end_file) (CONTEXT_t * C);

    void (*start_activity) (CONTENT_t * CONTENT);
    void (*end_activity) (CONTENT_t * CONTENT);

    void (*start_act) (CONTENT_t * CONTENT);
    void (*end_act) (CONTENT_t * CONTENT);

    void (*start_subject) (CONTENT_t * CONTENT);
    void (*end_subject) (CONTENT_t * CONTENT);

    void (*start_state) (CONTENT_t * CONTENT, char class, unsigned char prop, int nest, int repc);
    void (*end_state) (CONTENT_t * CONTENT, char class, success_t rc, int nest, int repc);

    void (*act) (CONTENT_t * CONTENT, elem_t * root);
    void (*subject) (CONTENT_t * CONTENT, elem_t * root);
    void (*attributes) (CONTENT_t * CONTENT, elem_t * root);

    void (*sep) (CONTEXT_t * C);
    void (*token) (CONTEXT_t * C, char token);
    void (*string) (CONTEXT_t * C, elem_t * branch);
    void (*frag) (CONTEXT_t * C, unsigned char len, unsigned char *frag);
} emit_t;

#define emit_initialize(C) \
    if (emit->initialize) {emit->initialize(C);}
#define emit_finalize(C) \
    if (emit->finalize) {emit->finalize(C);}

#define emit_start_file(C) \
    if (emit->start_file) {emit->start_file(C);}
#define emit_end_file(C) \
    if (emit->end_file) {emit->end_file(C);}

#define emit_start_activity(CONTENT) \
    if (emit->start_activity) {emit->start_activity(CONTENT);}
#define emit_end_activity(CONTENT) \
    if (emit->end_activity) {emit->end_activity(CONTENT);}

#define emit_start_act(CONTENT) \
    if (emit->start_act) {emit->start_act(CONTENT);}
#define emit_end_act(CONTENT) \
    if (emit->end_act) {emit->end_act(CONTENT);}

#define emit_start_subject(CONTENT) \
    if (emit->start_subject) {emit->start_subject(CONTENT);}
#define emit_end_subject(CONTENT) \
    if (emit->end_subject) {emit->end_subject(CONTENT);}

#define emit_start_state(C, class, prop, nest, repc) \
    if (emit->start_state) {emit->start_state(C, class, prop, nest, repc);}
#define emit_end_state(C, class, rc, nest, repc) \
    if (emit->end_state) {emit->end_state(C, class, rc, nest, repc);}
#define emit_act(CONTENT, root) \
    if (emit->act) {emit->act(CONTENT, root);}
#define emit_subject(CONTENT, root) \
    if (emit->subject) {emit->subject(CONTENT, root);}
#define emit_attributes(CONTENT, root) \
    if (emit->attributes) {emit->attributes(CONTENT, root);}

#define emit_sep(C) \
    if (emit->sep) {emit->sep(C);}
#define emit_token(C, token) \
    if (emit->token) {emit->token(C, token);}
#define emit_string(C, branch) \
    if (emit->string) {emit->string(C, branch);}
#define emit_frag(C, len, frag) \
    if (emit->frag) {emit->frag(C, len, frag);}

// if we're not providing the function in any api,
//    then we can avoid the runtime cost of testing for it
#undef emit_start_act
#define emit_start_act(CONTENT, len, frag)

#undef emit_end_act
#define emit_end_act(CONTENT, len, frag)

#undef emit_start_subject
#define emit_start_subject(CONTENT, len, frag)

#undef emit_end_subject
#define emit_end_subject(CONTENT, len, frag)

#undef emit_frag
#define emit_frag(C, len, frag)

// emit.c
extern emit_t *emit;
extern emit_t g_api, g1_api, g2_api, g3_api, t_api, t1_api, gv_api;
char je_char_prop(unsigned char prop, char noprop);
void je_append_token(CONTEXT_t *C, char **pos, char tok);
void je_append_string(CONTEXT_t *C, char **pos, char *string);
void je_append_ulong(CONTEXT_t *C, char **pos, uint64_t integer);
void je_append_runtime(CONTEXT_t *C, char **pos, uint64_t run_sec, uint64_t run_ns);
void je_emit_list(CONTEXT_t * C, FILE * chan, elem_t * subject);
void je_emit_ikea(CONTENT_t * CONTENT, elem_t *list);

#ifdef __cplusplus
}
#endif

#endif
