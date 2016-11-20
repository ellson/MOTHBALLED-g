/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <errno.h>
#include <assert.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_SYSINFO
#include <sys/sysinfo.h>
#else
#ifndef HAVE_CLOCK_GETTIME
#include <sys/time.h>
#endif
#endif

// private includes
#include "thread.h"
#include "process.h"
//
// public include
#include "libg_process.h"

void process(int *pargc, char *argv[], int optind, char needstats)
{
    PROCESS_t process;
    struct passwd *pw;
    struct utsname unamebuf;
    uid_t uid;
    pid_t pid;

#if defined(HAVE_CLOCK_GETTIME)
    // Y2038-unsafe struct - but should be ok for uptime
    // ref: https://sourceware.org/glibc/wiki/Y2038ProofnessDesign
    struct timespec uptime;   
    struct timespec starttime; 
    if (clock_gettime(CLOCK_BOOTTIME, &(uptime)))
        FATAL("clock_gettime()");
    process.uptime = uptime.tv_sec;
    process.uptime_nsec = uptime.tv_nsec;
    if (clock_gettime(CLOCK_REALTIME, &(starttime)))
        FATAL("clock_gettime()");
    process.starttime = starttime.tv_sec;
    process.starttime_nsec = starttime.tv_nsec;
#else
    // Y2038-unsafe struct - but should be ok for uptime
    // ref: https://sourceware.org/glibc/wiki/Y2038ProofnessDesign
    struct timeval uptime;   
    struct timeval starttime;   
    if (gettimeofday(&(uptime), NULL))
        FATAL("gettimeofday()");
    process.uptime = uptime.tv_sec;
    process.uptime_nsec = uptime.tv_usec * TEN3;
    if (gettimeofday(&starttime, NULL))
        FATAL("gettimeofday()");
    process.starttime = starttime.tv_sec;
    process.starttime_nsec = starttime.tv_usec * TEN3;
#endif

    process.progname = argv[0];
    pid = geteuid();
    process.pid = pid;
    uid = geteuid();
    if (!(pw = getpwuid(uid)))
        FATAL("getpwuid()");
    process.username = pw->pw_name;
    if (uname(&unamebuf))
        FATAL("uname()");
    process.hostname = unamebuf.nodename;
    process.osname = unamebuf.sysname;
    process.osrelease = unamebuf.release;
    process.osmachine = unamebuf.machine;
    process.needstats = needstats;

    // run a THREAD to process the input
    process.THREAD = thread(&process, pargc, argv, optind);
}