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

#ifndef THREAD_H
#define THREAD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"
#include "fatal.h"
#include "inbuf.h"
#include "list.h"
#include "grammar.h"
#include "token.h"
#include "identifier.h"
#include "vstring.h"
#include "ikea.h"
#include "iter.h"
#include "tree.h"
#include "parse.h"
#include "container.h"
#include "process.h"
#include "print.h"
#include "info.h"

#define TOKEN() ((TOKEN_t*)THREAD)
#define LIST() ((LIST_t*)THREAD)
#define INBUF() ((INBUF_t*)THREAD)

typedef void* (*out_open_fn_t)(void *descriptor, char *mode);
typedef size_t (*out_write_fn_t)(THREAD_t *THREAD, unsigned char *cp, size_t len);
typedef void (*out_flush_fn_t)(THREAD_t *THREAD);
typedef void (*out_close_fn_t)(THREAD_t *THREAD);

typedef struct {
    out_open_fn_t out_open_fn;
    out_write_fn_t out_write_fn;
    out_flush_fn_t out_flush_fn;
    out_close_fn_t out_close_fn;
} out_disc_t;

struct thread_s {
    TOKEN_t TOKEN;             // TOKEN context. May be cast from THREAD
    PROCESS_t *PROCESS;        // The PROCESS that started this THREAD
    ikea_store_t *ikea_store;  // persistency
    int style;                 // degree of friendliness in print outputs
    char sep;                  // the next separator
                               // (either 0, or ' ' if following a IDENTIFIER or VSTRING that
                               // requires a separator,  but may be ignored if
                               // the next character is a token which
                               // implicitly separates.)
 
    elem_t *identifiers;       // tree of identifiers    // FIXME - add mutex and move to PROCESS  ??

    unsigned char buf[1024];   // output buffering
    int pos;

    void *out;                 // output FILE* or ikea_box_t*
    out_disc_t *out_disc;

    char contenthash[128];     // big enough for content hash
                               // checked by assert in ikea_box_open()
    int pretty;                // 0 = mimimal sseparators,  !0 = pretty spacing separators

    // stats
    long stat_inactcount;
    long stat_containdepth;      
    long stat_containdepthmax;      
    long stat_outactcount;
};

THREAD_t * thread(PROCESS_t *PROCESS, int *pargc, char *argv[], int optind);

#ifdef __cplusplus
}
#endif

#endif
