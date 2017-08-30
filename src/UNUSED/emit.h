/* vim:set shiftwidth=4 ts=8 expandtab: */

/*************************************************************************
 * Copyright (c) 2017 AT&T Intellectual Property
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: John Ellson <john.ellson@gmail.com>
 *************************************************************************/

#ifndef EMIT_H
#define EMIT_H

#ifdef __cplusplus
extern "C" {
#endif

// emit styles
typedef enum {
     MINIMUM_SPACE_STYLE = 0,
     SHELL_FRIENDLY_STYLE = 1
} style_t;

typedef struct {
    char *name;
    void (*initialize) (CONTAINER_t * CONTAINER);
    void (*finalize) (CONTAINER_t * CONTAINER);

    void (*start_file) (CONTAINER_t * CONTAINER);
    void (*end_file) (CONTAINER_t * CONTAINER);

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

    void (*sep) (CONTAINER_t * CONTAINER);
    void (*token) (CONTAINER_t * CONTAINER, char token);
    void (*string) (CONTAINER_t * CONTAINER, elem_t * branch);
    void (*frag) (CONTAINER_t * CONTAINER, unsigned char len, unsigned char *frag);
} emit_t;

#define emit_initialize(CONTAINER) \
    if (emit->initialize) {emit->initialize(CONTAINER);}
#define emit_finalize(CONTAINER) \
    if (emit->finalize) {emit->finalize(CONTAINER);}

#define emit_start_file(CONTAINER) \
    if (emit->start_file) {emit->start_file(CONTAINER);}
#define emit_end_file(CONTAINER) \
    if (emit->end_file) {emit->end_file(CONTAINER);}

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

#define emit_start_state(CONTAINER, class, prop, nest, repc) \
    if (emit->start_state) {emit->start_state(CONTAINER, class, prop, nest, repc);}
#define emit_end_state(CONTAINER, class, rc, nest, repc) \
    if (emit->end_state) {emit->end_state(CONTAINER, class, rc, nest, repc);}
#define emit_act(CONTAINER, root) \
    if (emit->act) {emit->act(CONTAINER, root);}
#define emit_subject(CONTAINER, root) \
    if (emit->subject) {emit->subject(CONTAINER, root);}
#define emit_attributes(CONTAINER, root) \
    if (emit->attributes) {emit->attributes(CONTAINER, root);}

#define emit_sep(CONTAINER) \
    if (emit->sep) {emit->sep(CONTAINER);}
#define emit_token(CONTAINER, token) \
    if (emit->token) {emit->token(CONTAINER, token);}
#define emit_string(CONTAINER, branch) \
    if (emit->string) {emit->string(CONTAINER, branch);}
#define emit_frag(CONTAINER, len, frag) \
    if (emit->frag) {emit->frag(CONTAINER, len, frag);}

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
#define emit_frag(CONTAINER, len, frag)

// emit.c
extern emit_t *emit;
extern emit_t g_api, g1_api, g2_api, g3_api, t_api, t1_api, gv_api;
char char_prop(unsigned char prop, char noprop);
void append_token(CONTAINER_t * CONTAINER, char **pos, char tok);
void append_string(CONTAINER_t * CONTAINER, char **pos, char *string);
void append_ulong(CONTAINER_t * CONTAINER, char **pos, uint64_t integer);
void append_runtime(CONTAINER_t * CONTAINER, char **pos, uint64_t run_sec, uint64_t run_ns);
void je_emit_list(CONTAINER_t * CONTAINER, FILE * chan, elem_t * subject);
void je_emit_ikea(CONTAINER_t * CONTAINER, elem_t *list);

#ifdef __cplusplus
}
#endif

#endif
