/* vim:set shiftwidth=4 ts=8 expandtab: */

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
#include "ikea.h"
#include "tree.h"
#include "graph.h"
#include "container.h"
#include "session.h"
#include "print.h"
#include "info.h"

struct thread_s {
    TOKEN_t TOKEN;             // TOKEN context. May be cast from THREAD
    CONTAINER_t *CONTAINER;    // The top level CONTAINER in this THREAD
    SESSION_t *SESSION;        // The SESSION that started this THREAD
    ikea_store_t *ikea_store;  // persistency
    int style;                 // degree of friendliness in print outputs
    char sep;                  // the next separator
                               // (either 0, or ' ' if following a STRING that
                               // requires a separator,  but may be ignored if
                               // the next character is a token which
                               // implicitly separates.)
    uint64_t stat_containdepth;      
    uint64_t stat_containdepthmax;      
    uint64_t stat_containcount;      
};

// functions
THREAD_t * thread(SESSION_t *SESSION, int *pargc, char *argv[], int optind);

#ifdef __cplusplus
}
#endif

#endif
