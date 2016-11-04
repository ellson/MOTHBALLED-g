/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef THREAD_H
#define THREAD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "token.h"
#include "ikea.h"

typedef struct {               // THREAD context
    TOKEN_t TOKEN;             // TOKEN context.  Must be first to allow casting from THREAD
    SESSION_t *SESSION;        // SESSION context at the top level of nested containment

    int style;                 // degree of friendliness in print outputs
    char sep;                  // the next separator
                               // (either 0, or ' ' if following a STRING that
                               // requires a separator,  but may be ignored if
                               // the next character is a token which
                               // implicitly separates.)

    FILE *out;                 // typically stdout for parser debug outputs
    ikea_store_t *ikea_store;  // persistency
} THREAD_t;

#ifdef __cplusplus
}
#endif

#endif
