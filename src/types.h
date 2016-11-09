/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef TYPES_H
#define TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct session_s SESSION_t;

typedef struct thread_s THREAD_t;
typedef struct token_s TOKEN_t;
typedef struct list_s LIST_t;
typedef struct inbuf_s INBUF_t;

typedef struct elem_s elem_t;
typedef struct inbufelem_s inbufelem_t;

typedef struct container_s CONTAINER_t;
typedef struct parse_s PARSE_t;

typedef struct ikea_store_s ikea_store_t;
typedef struct ikea_box_s ikea_box_t;

typedef enum {
        SUCCESS,
        FAIL
} success_t;

#ifdef __cplusplus
}
#endif

#endif
