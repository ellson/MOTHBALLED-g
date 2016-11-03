/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <errno.h>
#include <assert.h>

// include local configuration
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

#include "info.h"

#define SESSION_BUF_SIZE 1024

#define As(attr,valu) { \
     append_string  (GRAPH, &pos, attr); \
     append_token   (GRAPH, &pos, '='); \
     append_string  (GRAPH, &pos, valu); \
}
#define Au(attr,valu) { \
     append_string  (GRAPH, &pos, attr); \
     append_token   (GRAPH, &pos, '='); \
     append_ulong  (GRAPH, &pos, valu); \
}
#define Ar(attr,valu_s,valu_ns) { \
     append_string  (GRAPH, &pos, attr); \
     append_token   (GRAPH, &pos, '='); \
     append_runtime  (GRAPH, &pos, valu_s, valu_ns); \
}

/**
 * This code collects info from the environment to:
 *  - populate session info into attributes of a 'g' NODE
 *  - provide a readable unique session name for session-freeze tar file names
 *  - capture start time for stats.
 *
 * This code is very likely Linux specific, and may need canoditionals
 * for porting to other OS.
 *
 * The resulting info is collected into a buffer using minimal
 * spacing g format
 * e.g.      "stats_fixed[progname=g username=ellson hostname=work .... ]
 *
 * There is an attribute pretty-printer function for when this
 * is printed for the user.
 *
 * This info is gathered just once, so a statically sized buffer
 * is used. It is only filled on the first call,  if session() is
 * called again the same result is used.
 *
 * @param SESSION context
 * @return formatted string result
 */
char * session(SESSION_t * SESSION)
{
    GRAPH_t *GRAPH = SESSION->GRAPH;
    static char buf[SESSION_BUF_SIZE];
    static char *pos = &buf[0];  // NB. static. This initalization happens only once
    static struct passwd *pw;
    static struct utsname unamebuf;
    static uid_t uid;
#ifdef HAVE_CLOCK_GETTIME
    // Y2038-unsafe - FIXME
    static struct timespec starttime;
#else
    // Y2038-unsafe - FIXME
    static struct timeval starttime;
#endif

    if (pos != &buf[0]) { // have we been here before?
        return buf;
    }

    // gather info
    
    SESSION->pid = getpid();
    uid = geteuid();
    if (!(pw = getpwuid(uid)))
        FATAL("getpwuid()");
    SESSION->username = pw->pw_name;
    if (uname(&unamebuf))
        FATAL("uname()");
    SESSION->hostname = unamebuf.nodename;
    SESSION->osname = unamebuf.sysname;
    SESSION->osrelease = unamebuf.release;
    SESSION->osmachine = unamebuf.machine;
#if defined(HAVE_CLOCK_GETTIME)
    // Y2038-unsafe function - but should be ok for uptime and starttime
    // ref: https://sourceware.org/glibc/wiki/Y2038ProofnessDesign
    if (clock_gettime(CLOCK_BOOTTIME, &(SESSION->uptime)))
        FATAL("clock_gettime()");
    if (clock_gettime(CLOCK_REALTIME, &starttime))
        FATAL("clock_gettime()");
#else
    // Y2038-unsafe function - but should be ok for uptime and starttime
    // ref: htt,ps://sourceware.org/glibc/wiki/Y2038ProofnessDesign
    if (gettimeofday(&(SESSION->uptime), NULL))
        FATAL("gettimeofday()");
    if (gettimeofday(&starttime, NULL))
        FATAL("gettimeofday()");
#endif

    // write in canonical g format
    // - minimal spacing - one SUBJECT per line
    // - alpha sorted nodes and attributes

    GRAPH->sep = '\0';
    append_string  (GRAPH, &pos, "stats_fixed");
    append_token   (GRAPH, &pos, '[');
    Au("elemmallocsize",        LISTALLOCNUM * sizeof(elem_t));
    Au("elempermalloc",         LISTALLOCNUM);
    Au("elemsize",              sizeof(elem_t));
    As("hostname",              SESSION->hostname);
    Au("inbufcapacity",         INBUFIZE);
    Au("inbufmallocsize",       INBUFALLOCNUM * sizeof(inbuf_t));
    Au("inbufpermalloc",        INBUFALLOCNUM);
    Au("inbufsize",             sizeof(inbuf_t));
    As("osmachine",             SESSION->osmachine);
    As("osname",                SESSION->osname);
    As("osrelease",             SESSION->osrelease);
    Au("pid",                   SESSION->pid);
    As("progname",              SESSION->progname);
    Au("starttime",             starttime.tv_sec);
    Au("uptime",                SESSION->uptime.tv_sec);
    As("username",              SESSION->username);
    Au("voidptrsize",           sizeof(void*));
    append_token   (GRAPH, &pos, ']');

    assert(pos < buf+SESSION_BUF_SIZE);
    return buf;
}

#define STATS_BUF_SIZE 2048
#define TEN9 1000000000
#define TEN3 1000

/**
 * format running stats as a string
 *
 * @param SESSION context
 * @return formatted string
 */
char * stats(SESSION_t * SESSION)
{
    GRAPH_t *GRAPH = SESSION->GRAPH;
    TOKEN_t *TOKEN = (TOKEN_t*)GRAPH;
    LIST_t *LIST = (LIST_t*)GRAPH;
    INBUF_t *INBUF = (INBUF_t*)GRAPH;
    static char buf[STATS_BUF_SIZE];
    uint64_t runtime, lend, itot, etot;

    char *pos = &buf[0];  // NB non-static.  stats are updated and re-formatted on each call
    
#ifdef HAVE_CLOCK_GETTIME
    // Y2038-unsafe struct - but should be ok for uptime
    // ref: https://sourceware.org/glibc/wiki/Y2038ProofnessDesign
    static struct timespec uptime;
    static struct timespec nowtime;
#else
    // Y2038-unsafe struct - but should be ok for uptime
    // ref: https://sourceware.org/glibc/wiki/Y2038ProofnessDesign
    static struct timeval nowtime;
#endif

    // gather info
    
#if defined(HAVE_CLOCK_GETTIME)
    // Y2038-unsafe struct - but should be ok for uptime
    // ref: https://sourceware.org/glibc/wiki/Y2038ProofnessDesign
    if (clock_gettime(CLOCK_BOOTTIME, &uptime))
        FATAL("clock_gettime()");
    if (clock_gettime(CLOCK_REALTIME, &nowtime))
        FATAL("clock_gettime()");
    runtime = ((uint64_t)uptime.tv_sec * TEN9 + (uint64_t)uptime.tv_nsec)
            - ((uint64_t)(SESSION->uptime.tv_sec) * TEN9 + (uint64_t)(SESSION->uptime.tv_nsec));
#else
    // Y2038-unsafe struct - but should be ok for uptime
    // ref: https://sourceware.org/glibc/wiki/Y2038ProofnessDesign
    if (gettimeofday(&nowtime, NULL))
        FATAL("gettimeofday()");
    runtime = ((uint64_t)nowtime.tv_sec * TEN9 + (uint64_t)nowtime.tv_usec) * TEN3
            - ((uint64_t)(SESSION->uptime.tv_sec) * TEN9 + (uint64_t)(SESSION->uptime.tv_usec) * TEN3);
#endif
    itot = INBUF->stat_inbufmalloc * INBUFALLOCNUM * sizeof(inbuf_t);
    etot = LIST->stat_elemmalloc * LISTALLOCNUM * sizeof(elem_t);
    lend = (TOKEN->stat_lfcount ? TOKEN->stat_lfcount : TOKEN->stat_crcount);
  
    // write in canonical g format
    // - minimal spacing - one SUBJECT per line
    // - alpha sorted nodes and attributes

    GRAPH->sep = '\0';
    append_string  (GRAPH, &pos, "stats_running");
    append_token   (GRAPH, &pos, '[');
    Au("charpersecond",         TOKEN->stat_incharcount+TEN9/runtime);
    Au("elemmalloccount",       LIST->stat_elemmalloc);
    Au("elemmalloctotal",       etot);
    Au("elemmax",               LIST->stat_elemmax);
    Au("elemnow",               LIST->stat_elemnow);
    Au("fragmax",               LIST->stat_fragmax);
    Au("fragnow",               LIST->stat_fragnow);
    Au("inbufmalloccount",      INBUF->stat_inbufmalloc);
    Au("inbufmalloctotal",      itot);
    Au("inbufmax",              INBUF->stat_inbufmax);
    Au("inbufnow",              INBUF->stat_inbufnow);
    Au("inactcount",            GRAPH->stat_inactcount);
    Au("inactpatterns",         GRAPH->stat_patternactcount);
    Au("inactnonpatterns",      GRAPH->stat_nonpatternactcount);
    Au("inactspersecond",       GRAPH->stat_inactcount*TEN9/runtime);
    Au("incharcount",           TOKEN->stat_incharcount);
    Au("infragcount",           TOKEN->stat_infragcount);
    Au("infilecount",           TOKEN->stat_infilecount);
    Au("inlinecount",           lend + 1);
    Au("instringcount",         TOKEN->stat_instringcount);
    Au("malloctotal",           itot+etot);
    Au("nowtime",               nowtime.tv_sec);
    Au("outactcount",           GRAPH->stat_outactcount);
    Au("patternmatches",        GRAPH->stat_patternmatches);
    Ar("runtime",               runtime/TEN9, runtime%TEN9);
    Au("sameas",                GRAPH->stat_sameas);
    append_token   (GRAPH, &pos, ']');

    assert(pos < buf+STATS_BUF_SIZE);
    return buf;
}
