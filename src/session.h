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

struct session_s {
    THREAD_t *THREAD;          // THREADs in this SESSION
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
