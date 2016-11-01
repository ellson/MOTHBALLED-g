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
     append_string  (PARSE, &pos, attr); \
     append_token   (PARSE, &pos, '='); \
     append_string  (PARSE, &pos, valu); \
}
#define Au(attr,valu) { \
     append_string  (PARSE, &pos, attr); \
     append_token   (PARSE, &pos, '='); \
     append_ulong  (PARSE, &pos, valu); \
}
#define Ar(attr,valu_s,valu_ns) { \
     append_string  (PARSE, &pos, attr); \
     append_token   (PARSE, &pos, '='); \
     append_runtime  (PARSE, &pos, valu_s, valu_ns); \
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
 * @param PARSE context
 * @return formatted string result
 */
char * session(PARSE_t * PARSE)
{
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

    append_string  (PARSE, &pos, "stats_fixed");
    append_token   (PARSE, &pos, '[');

    As("progname",              PARSE->progname);

    PARSE->pid = getpid();
    Au("pid",                   PARSE->pid);

    uid = geteuid();
    if (!(pw = getpwuid(uid)))
        FATAL("getpwuid()");

    PARSE->username = pw->pw_name;
    As("username",              PARSE->username);

    if (uname(&unamebuf))
        FATAL("uname()");
    PARSE->hostname = unamebuf.nodename;
    PARSE->osname = unamebuf.sysname;
    PARSE->osrelease = unamebuf.release;
    PARSE->osmachine = unamebuf.machine;
    As("hostname",              PARSE->hostname);
    As("osname",                PARSE->osname);
    As("osrelease",             PARSE->osrelease);
    As("osmachine",             PARSE->osmachine);

#if defined(HAVE_CLOCK_GETTIME)
    // Y2038-unsafe function - but should be ok for uptime and starttime
    // ref: https://sourceware.org/glibc/wiki/Y2038ProofnessDesign
    if (clock_gettime(CLOCK_BOOTTIME, &(PARSE->uptime)))
        FATAL("clock_gettime()");
    if (clock_gettime(CLOCK_REALTIME, &starttime))
        FATAL("clock_gettime()");
#else
    // Y2038-unsafe function - but should be ok for uptime and starttime
    // ref: htt,ps://sourceware.org/glibc/wiki/Y2038ProofnessDesign
    if (gettimeofday(&(PARSE->uptime), NULL))
        FATAL("gettimeofday()");
    if (gettimeofday(&starttime, NULL))
        FATAL("gettimeofday()");
#endif
    Au("uptime",                PARSE->uptime.tv_sec);
    Au("starttime",             starttime.tv_sec);
    Au("voidptrsize",           sizeof(void*));
    Au("inbufsize",             sizeof(inbuf_t));
    Au("inbufcapacity",         INBUFIZE);
    Au("inbuf_per_malloc",      INBUFALLOCNUM);
    Au("inbufmallocsize",       INBUFALLOCNUM * sizeof(inbuf_t));
    Au("elemsize",              sizeof(elem_t));
    Au("elem_per_malloc",       LISTALLOCNUM);
    Au("elemmallocsize",        LISTALLOCNUM * sizeof(elem_t));

    append_token   (PARSE, &pos, ']');

    assert(pos < buf+SESSION_BUF_SIZE);

    return buf;
}

#define STATS_BUF_SIZE 2048
#define TEN9 1000000000
#define TEN3 1000

/**
 * format running stats as a string
 *
 * @param PARSE context
 * @return formatted string
 */
char * stats(PARSE_t * PARSE)
{
    TOKEN_t *TOKEN = (TOKEN_t*)PARSE;
    LIST_t *LIST = (LIST_t*)PARSE;
    INBUF_t *INBUF = (INBUF_t*)PARSE;
    static char buf[STATS_BUF_SIZE];
    uint64_t runtime, itot, etot;

    char *pos = &buf[0];  // NB non-static.  stats are updated and re-formatted on each call
    
    PARSE->sep = '\0';
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

    append_string  (PARSE, &pos, "stats_running");
    append_token   (PARSE, &pos, '[');

#if defined(HAVE_CLOCK_GETTIME)
    // Y2038-unsafe struct - but should be ok for uptime
    // ref: https://sourceware.org/glibc/wiki/Y2038ProofnessDesign
    if (clock_gettime(CLOCK_BOOTTIME, &uptime))
        FATAL("clock_gettime()");
    if (clock_gettime(CLOCK_REALTIME, &nowtime))
        FATAL("clock_gettime()");
    runtime = ((uint64_t)uptime.tv_sec * TEN9 + (uint64_t)uptime.tv_nsec)
            - ((uint64_t)(PARSE->uptime.tv_sec) * TEN9 + (uint64_t)(PARSE->uptime.tv_nsec));
#else
    // Y2038-unsafe struct - but should be ok for uptime
    // ref: https://sourceware.org/glibc/wiki/Y2038ProofnessDesign
    if (gettimeofday(&nowtime, NULL))
        FATAL("gettimeofday()");
    runtime = ((uint64_t)nowtime.tv_sec * TEN9 + (uint64_t)nowtime.tv_usec) * TEN3
            - ((uint64_t)(PARSE->uptime.tv_sec) * TEN9 + (uint64_t)(PARSE->uptime.tv_usec) * TEN3);
#endif
    Au("nowtime",               nowtime.tv_sec);
    Ar("runtime",               runtime/TEN9, runtime%TEN9);
    Au("filecount",             PARSE->TOKEN.stat_filecount);
    Au("linecount",             1 + (TOKEN->stat_lfcount ? TOKEN->stat_lfcount : TOKEN->stat_crcount));
    Au("inactcount",            PARSE->stat_inactcount);
    Au("inacts_per_second",     PARSE->stat_inactcount*TEN9/runtime);
    Au("sameas",                PARSE->stat_sameas);
    Au("patternacts",           PARSE->stat_patternactcount);
    Au("nonpatternacts",        PARSE->stat_nonpatternactcount);
    Au("patternmatches",        PARSE->stat_patternmatches);
    Au("outactcount",           PARSE->stat_outactcount);
    Au("stringcount",           TOKEN->stat_stringcount);
    Au("fragcount",             TOKEN->stat_fragcount);
    Au("charcount",             TOKEN->stat_inchars);
    Au("char_per_second",       TOKEN->stat_inchars + TEN9 / runtime);
    Au("inbufmax",              INBUF->stat_inbufmax);
    Au("inbufnow",              INBUF->stat_inbufnow);
    Au("elemmax",               LIST->stat_elemmax);
    Au("elemnow",               LIST->stat_elemnow);
    Au("inbufmalloccount",      INBUF->stat_inbufmalloc);

    itot = INBUF->stat_inbufmalloc * INBUFALLOCNUM * sizeof(inbuf_t);
    Au("inbufmalloctotal",      itot);
    Au("elemmalloccount",       LIST->stat_elemmalloc);

    etot = LIST->stat_elemmalloc * LISTALLOCNUM * sizeof(elem_t);
    Au("elemmalloctotal",       etot);
    Au("malloctotal",           itot+etot);

    append_token   (PARSE, &pos, ']');

    assert(pos < buf+STATS_BUF_SIZE);

    return buf;
}
