/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef THREAD_H
#define THREAD_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct graph_s GRAPH_t;
typedef struct container_s CONTAINER_t;
typedef struct thread_s THREAD_t;
typedef struct session_s SESSION_t;

#include "fatal.h"
#include "inbuf.h"
#include "list.h"
#include "success.h"
#include "grammar.h"
#include "token.h"
#include "ikea.h"
#include "graph.h"
#include "container.h"
#include "session.h"

struct thread_s {
    TOKEN_t TOKEN;             // TOKEN context. May be cast from THREAD
    CONTAINER_t *CONTAINER;    // The top level CONTAINER in this THREAD
    SESSION_t *SESSION;        // The SESSION that started this THREAD
    FILE *out;                 // typically stdout for parser debug outputs
    ikea_store_t *ikea_store;  // persistency
    int style;                 // degree of friendliness in print outputs
    char sep;                  // the next separator
                               // (either 0, or ' ' if following a STRING that
                               // requires a separator,  but may be ignored if
                               // the next character is a token which
                               // implicitly separates.)
};

// functions
void thread(SESSION_t *SESSION, int *pargc, char *argv[], int optind);

#ifdef __cplusplus
}
#endif

#endif
