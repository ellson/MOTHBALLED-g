/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef SESSION_H
#define SESSION_H

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

#include "container.h"
#include "ikea.h"

struct session_s {
    CONTAINER_t *CONTAINER;    // A top level CONTAINER
    FILE *out;                 // typically stdout for parser debug outputs
    style_t style;             // spacing style in emitted outputs
                               
    ikea_store_t *ikea_store;  // persistency
    ikea_box_t *namehash_buckets[64];

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
