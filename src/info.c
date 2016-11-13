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

#include "thread.h"
#include "session.h"
#include "info.h"

#define BUF_SIZE 2048

// Runtime values  (sec.ns)
#define Ar(attr,valu_s,valu_ns) { \
     append_string  (THREAD, &pos, attr); \
     append_token   (THREAD, &pos, '='); \
     append_runtime (THREAD, &pos, valu_s, valu_ns); \
}
// String values
#define As(attr,valu) { \
     append_string  (THREAD, &pos, attr); \
     append_token   (THREAD, &pos, '='); \
     append_string  (THREAD, &pos, valu); \
}
// Integer values
#define Au(attr,valu) { \
     append_string  (THREAD, &pos, attr); \
     append_token   (THREAD, &pos, '='); \
     append_ulong   (THREAD, &pos, valu); \
}

/**
 * The info is formatted into a buffer using minimal
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
 * @param CONTAINER context
 * @return formatted string result
 */
char * info_session(CONTAINER_t * CONTAINER)
{
    THREAD_t *THREAD = CONTAINER->THREAD;
    SESSION_t *SESSION = THREAD->SESSION;

// FIXME - can't do this in threaded code!!
    static char buf[BUF_SIZE];
    static char *pos = &buf[0];  // NB. static. This initalization happens only once

    if (pos != &buf[0]) { // have we been here before?
        return buf;
    }

    // write in canonical g format
    // - minimal spacing - one SUBJECT per line
    // - alpha sorted nodes and attributes

    THREAD->sep = '\0';
    append_string  (THREAD, &pos, "stats_fixed");
    append_token   (THREAD, &pos, '[');
    Au("elemmallocsize",        LISTALLOCNUM * sizeof(elem_t));
    Au("elempermalloc",         LISTALLOCNUM);
    Au("elemsize",              sizeof(elem_t));
    As("hostname",              SESSION->hostname);
    Au("inbufcapacity",         INBUFIZE);
    Au("inbufmallocsize",       INBUFALLOCNUM * sizeof(inbufelem_t));
    Au("inbufpermalloc",        INBUFALLOCNUM);
    Au("inbufsize",             sizeof(inbufelem_t));
    As("osmachine",             SESSION->osmachine);
    As("osname",                SESSION->osname);
    As("osrelease",             SESSION->osrelease);
    Au("pid",                   SESSION->pid);
    As("progname",              SESSION->progname);
    Au("starttime",             SESSION->starttime);
    Au("uptime",                SESSION->uptime);
    As("username",              SESSION->username);
    Au("voidptrsize",           sizeof(void*));
    append_token   (THREAD, &pos, ']');

    assert(pos < buf+BUF_SIZE);
    return buf;
}


/**
 * format running stats as a string
 *
 * @param CONTAINER context
 * @return formatted string
 */
char * info_stats(CONTAINER_t * CONTAINER)
{
    THREAD_t *THREAD = CONTAINER->THREAD;
    SESSION_t *SESSION = THREAD->SESSION;
    uint64_t runtime, lend, itot, etot, istr;
    char percent[6];

// FIXME - can't do this in threaded code!!
    static char buf[BUF_SIZE];
    char *pos = &buf[0];  // NB non-static.  stats are updated and re-formatted on each call
    
#ifdef HAVE_CLOCK_GETTIME
    // Y2038-unsafe struct - but should be ok for uptime
    // ref: https://sourceware.org/glibc/wiki/Y2038ProofnessDesign
    static struct timespec uptime;
    static struct timespec nowtime;
    if (clock_gettime(CLOCK_BOOTTIME, &uptime))
        FATAL("clock_gettime()");
    if (clock_gettime(CLOCK_REALTIME, &nowtime))
        FATAL("clock_gettime()");
    runtime = ((uint64_t)uptime.tv_sec * TEN9 + (uint64_t)uptime.tv_nsec)
            - (SESSION->uptime * TEN9 + SESSION->uptime_nsec);
#else
    // Y2038-unsafe struct - but should be ok for uptime
    // ref: https://sourceware.org/glibc/wiki/Y2038ProofnessDesign
    static struct timeval nowtime;
    if (gettimeofday(&nowtime, NULL))
        FATAL("gettimeofday()");
    runtime = ((uint64_t)nowtime.tv_sec * TEN9 + (uint64_t)nowtime.tv_usec) * TEN3
            - (SESSION->uptime * TEN9 + SESSION->uptime_nsec);
#endif

    itot = INBUF()->stat_inbufmalloc * INBUFALLOCNUM * sizeof(inbufelem_t);
    etot = LIST()->stat_elemmalloc * LISTALLOCNUM * sizeof(elem_t);
    lend = (TOKEN()->stat_lfcount ? TOKEN()->stat_lfcount : TOKEN()->stat_crcount);
    istr = TOKEN()->stat_instringshort + TOKEN()->stat_instringlong; 
    sprintf(percent,"%ld%%", (long)(TOKEN()->stat_instringshort * 100)/ istr);
  
    // write in canonical g format
    // - minimal spacing - one SUBJECT per line
    // - alpha sorted nodes and attributes

    THREAD->sep = '\0';
    append_string  (THREAD, &pos, "stats_running");
    append_token   (THREAD, &pos, '[');
    Au("charpersecond",         TOKEN()->stat_incharcount+TEN9/runtime);
    Au("containercount",        CONTAINER->stat_containercount);
    Au("containdepth",          THREAD->stat_containdepth);
    Au("containdepthmax",       THREAD->stat_containdepthmax);
    Au("elemmalloccount",       LIST()->stat_elemmalloc);
    Au("elemmalloctotal",       etot);
    Au("elemmax",               LIST()->stat_elemmax);
    Au("elemnow",               LIST()->stat_elemnow);
    Au("fragmax",               LIST()->stat_fragmax);
    Au("fragnow",               LIST()->stat_fragnow);
    Au("inbufmalloccount",      INBUF()->stat_inbufmalloc);
    Au("inbufmalloctotal",      itot);
    Au("inbufmax",              INBUF()->stat_inbufmax);
    Au("inbufnow",              INBUF()->stat_inbufnow);
    Au("inactcount",            CONTAINER->stat_inactcount);
    Au("inactedgepatterns",     CONTAINER->stat_patternedgecount);
    Au("inactnodepatterns",     CONTAINER->stat_patternnodecount);
    Au("inactnonpatterns",      CONTAINER->stat_nonpatternactcount);
    Au("inactpatternmatches",   CONTAINER->stat_patternmatches);
    Au("inactspersecond",       CONTAINER->stat_inactcount*TEN9/runtime);
    Au("incharcount",           TOKEN()->stat_incharcount);
    Au("infragcount",           TOKEN()->stat_infragcount);
    Au("infilecount",           TOKEN()->stat_infilecount);
    Au("inlinecount",           lend + 1);
    Au("instringcount",         istr);
    As("instringshort",         percent);
    Au("malloctotal",           itot+etot);
    Au("nowtime",               nowtime.tv_sec);
    Au("outactcount",           CONTAINER->stat_outactcount);
    Ar("runtime",               runtime/TEN9, runtime%TEN9);
    Au("sameas",                CONTAINER->stat_sameas);
    append_token   (THREAD, &pos, ']');

    assert(pos < buf+BUF_SIZE);
    return buf;
}
