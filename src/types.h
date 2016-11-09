/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef TYPES_H
#define TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct session_s SESSION_t;       // session.h

// THREAD_t TOKEN_t LIST_t INBUF_t are inter-castable
//   from since all 4 structs share the same start
typedef struct thread_s THREAD_t;         // thread.h
typedef struct token_s TOKEN_t;           // token.h
typedef struct list_s LIST_t;             // list.h
typedef struct inbuf_s INBUF_t;           // inbuf.h

typedef struct elem_s elem_t;             // list.h
typedef struct inbufelem_s inbufelem_t;   // inbuf.h

typedef struct container_s CONTAINER_t;   // container.h

typedef struct ikea_store_s ikea_store_t; // ikea.h
typedef struct ikea_box_s ikea_box_t;     // ikea.h

typedef enum {
        SUCCESS,
        FAIL
} success_t;

#ifdef __cplusplus
}
#endif

#endif
