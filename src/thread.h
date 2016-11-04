/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef THREAD_H
#define THREAD_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#ifdef HAVE_SYSINFO
#include <sys/sysinfo.h>
#else
#ifndef HAVE_CLOCK_GETTIME
#include <sys/time.h>
#endif
#endif

typedef struct graph_s GRAPH_t;
typedef struct container_s CONTAINER_t;
typedef struct thread_s THREAD_t;

#include "fatal.h"
#include "inbuf.h"
#include "list.h"
#include "success.h"
#include "grammar.h"
#include "token.h"
#include "ikea.h"
#include "graph.h"
#include "container.h"

struct thread_s {
    TOKEN_t TOKEN;             // TOKEN context.  Must be first to allow casting from THREAD

    CONTAINER_t *CONTAINER;    // A top level CONTAINER
    FILE *out;                 // typically stdout for parser debug outputs
                               
    ikea_store_t *ikea_store;  // persistency

    char needstats;            // flag set if -s on command line
    char *progname;            // name of program
    char *username;            // set by first call to g_session
    char *hostname;          
    char *osname;
    char *osrelease;
    char *osmachine;

#if defined(HAVE_CLOCK_GETTIME)
    // Y2038-unsafe struct - but should be ok for uptime
    // ref: https://sourceware.org/glibc/wiki/Y2038ProofnessDesign
    struct timespec uptime;     // time with subsec resolution since boot, used as the base for runtime calculations
#else
    // Y2038-unsafe struct - but should be ok for uptime
    // ref: https://sourceware.org/glibc/wiki/Y2038ProofnessDesign
    struct timeval uptime;      // time with subsec resolution since boot, used as the base for runtime calculations
#endif
    pid_t pid;
};

#ifdef __cplusplus
}
#endif

#endif
