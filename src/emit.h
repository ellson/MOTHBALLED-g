/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef EMIT_H
#define EMIT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "thread.h"

// emit styles
typedef enum {
     MINIMUM_SPACE_STYLE = 0,
     SHELL_FRIENDLY_STYLE = 1
} style_t;

typedef struct {
    char *name;
    void (*initialize) (GRAPH_t * GRAPH);
    void (*finalize) (GRAPH_t * GRAPH);

    void (*start_file) (GRAPH_t * GRAPH);
    void (*end_file) (GRAPH_t * GRAPH);

    void (*start_activity) (CONTAINER_t * CONTAINER);
    void (*end_activity) (CONTAINER_t * CONTAINER);

    void (*start_act) (CONTAINER_t * CONTAINER);
    void (*end_act) (CONTAINER_t * CONTAINER);

    void (*start_subject) (CONTAINER_t * CONTAINER);
    void (*end_subject) (CONTAINER_t * CONTAINER);

    void (*start_state) (CONTAINER_t * CONTAINER, char class, unsigned char prop, int nest, int repc);
    void (*end_state) (CONTAINER_t * CONTAINER, char class, success_t rc, int nest, int repc);

    void (*act) (CONTAINER_t * CONTAINER, elem_t * root);
    void (*subject) (CONTAINER_t * CONTAINER, elem_t * root);
    void (*attributes) (CONTAINER_t * CONTAINER, elem_t * root);

    void (*sep) (GRAPH_t * GRAPH);
    void (*token) (GRAPH_t * GRAPH, char token);
    void (*string) (GRAPH_t * GRAPH, elem_t * branch);
    void (*frag) (GRAPH_t * GRAPH, unsigned char len, unsigned char *frag);
} emit_t;

#define emit_initialize(GRAPH) \
    if (emit->initialize) {emit->initialize(GRAPH);}
#define emit_finalize(GRAPH) \
    if (emit->finalize) {emit->finalize(GRAPH);}

#define emit_start_file(GRAPH) \
    if (emit->start_file) {emit->start_file(GRAPH);}
#define emit_end_file(GRAPH) \
    if (emit->end_file) {emit->end_file(GRAPH);}

#define emit_start_activity(CONTAINER) \
    if (emit->start_activity) {emit->start_activity(CONTAINER);}
#define emit_end_activity(CONTAINER) \
    if (emit->end_activity) {emit->end_activity(CONTAINER);}

#define emit_start_act(CONTAINER) \
    if (emit->start_act) {emit->start_act(CONTAINER);}
#define emit_end_act(CONTAINER) \
    if (emit->end_act) {emit->end_act(CONTAINER);}

#define emit_start_subject(CONTAINER) \
    if (emit->start_subject) {emit->start_subject(CONTAINER);}
#define emit_end_subject(CONTAINER) \
    if (emit->end_subject) {emit->end_subject(CONTAINER);}

#define emit_start_state(GRAPH, class, prop, nest, repc) \
    if (emit->start_state) {emit->start_state(GRAPH, class, prop, nest, repc);}
#define emit_end_state(GRAPH, class, rc, nest, repc) \
    if (emit->end_state) {emit->end_state(GRAPH, class, rc, nest, repc);}
#define emit_act(CONTAINER, root) \
    if (emit->act) {emit->act(CONTAINER, root);}
#define emit_subject(CONTAINER, root) \
    if (emit->subject) {emit->subject(CONTAINER, root);}
#define emit_attributes(CONTAINER, root) \
    if (emit->attributes) {emit->attributes(CONTAINER, root);}

#define emit_sep(GRAPH) \
    if (emit->sep) {emit->sep(GRAPH);}
#define emit_token(GRAPH, token) \
    if (emit->token) {emit->token(GRAPH, token);}
#define emit_string(GRAPH, branch) \
    if (emit->string) {emit->string(GRAPH, branch);}
#define emit_frag(GRAPH, len, frag) \
    if (emit->frag) {emit->frag(GRAPH, len, frag);}

// if we're not providing the function in any api,
//    then we can avoid the runtime cost of testing for it
#undef emit_start_act
#define emit_start_act(CONTAINER, len, frag)

#undef emit_end_act
#define emit_end_act(CONTAINER, len, frag)

#undef emit_start_subject
#define emit_start_subject(CONTAINER, len, frag)

#undef emit_end_subject
#define emit_end_subject(CONTAINER, len, frag)

#undef emit_frag
#define emit_frag(GRAPH, len, frag)

// emit.c
extern emit_t *emit;
extern emit_t g_api, g1_api, g2_api, g3_api, t_api, t1_api, gv_api;
char char_prop(unsigned char prop, char noprop);
void append_token(GRAPH_t * GRAPH, char **pos, char tok);
void append_string(GRAPH_t * GRAPH, char **pos, char *string);
void append_ulong(GRAPH_t * GRAPH, char **pos, uint64_t integer);
void append_runtime(GRAPH_t * GRAPH, char **pos, uint64_t run_sec, uint64_t run_ns);
void je_emit_list(GRAPH_t * GRAPH, FILE * chan, elem_t * subject);
void je_emit_ikea(CONTAINER_t * CONTAINER, elem_t *list);

#ifdef __cplusplus
}
#endif

#endif
