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
#include "parse.h"
#include "container.h"
#include "session.h"
#include "print.h"
#include "info.h"

#define TOKEN() ((TOKEN_t*)THREAD)
#define LIST() ((LIST_t*)THREAD)
#define INBUF() ((INBUF_t*)THREAD)


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
 
    elem_t *attrid;            // tree of attrid strings;

    // stats
    long stat_containdepth;      
    long stat_containdepthmax;      
};

THREAD_t * thread(SESSION_t *SESSION, int *pargc, char *argv[], int optind);

#ifdef __cplusplus
}
#endif

#endif
