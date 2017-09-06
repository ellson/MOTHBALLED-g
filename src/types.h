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

#ifndef TYPES_H
#define TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct process_s PROCESS_t;       // session.h

// THREAD_t TOKEN_t IO_t LIST_t INBUF_t are inter-castable
//   from since all 5 structs share the same start
typedef struct thread_s THREAD_t;         // thread.h
typedef struct token_s TOKEN_t;           // token.h
typedef struct io_s IO_t;                 // input.h
typedef struct list_s LIST_t;             // list.h
typedef struct inbuf_s INBUF_t;           // inbuf.h

typedef struct elem_s elem_t;             // list.h
typedef struct inbufelem_s inbufelem_t;   // inbuf.h

typedef struct container_s CONTAINER_t;   // container.h

typedef struct ikea_store_s ikea_store_t; // ikea.h
typedef struct ikea_box_s ikea_box_t;     // ikea.h

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

typedef enum {
        SUCCESS,
        FAIL
} success_t;

#ifdef __cplusplus
}
#endif

#endif
