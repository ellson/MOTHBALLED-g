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

    append_string  (PARSE, &pos, "progname");
    append_token   (PARSE, &pos, '=');
    append_string  (PARSE, &pos, PARSE->progname);

    PARSE->pid = getpid();
    append_string  (PARSE, &pos, "pid");
    append_token   (PARSE, &pos, '=');
    append_ulong   (PARSE, &pos, PARSE->pid);

    uid = geteuid();
    if (!(pw = getpwuid(uid)))
        fatal_perror("Error - getpwuid(): ");
    PARSE->username = pw->pw_name;
    append_string  (PARSE, &pos, "username");
    append_token   (PARSE, &pos, '=');
    append_string  (PARSE, &pos, PARSE->username);

    if (uname(&unamebuf))
        fatal_perror("Error - uname(): ");
    PARSE->hostname = unamebuf.nodename;
    append_string  (PARSE, &pos, "hostname");
    append_token   (PARSE, &pos, '=');
    append_string  (PARSE, &pos, PARSE->hostname);

#if defined(HAVE_CLOCK_GETTIME)
    // Y2038-unsafe function - but should be ok for uptime
    // ref: https://sourceware.org/glibc/wiki/Y2038ProofnessDesign
    if (clock_gettime(CLOCK_BOOTTIME, &(PARSE->uptime)))
        fatal_perror("Error - clock_gettime(): ");
    append_string  (PARSE, &pos, "uptime");
    append_token   (PARSE, &pos, '=');
    append_ulong   (PARSE, &pos, PARSE->uptime.tv_sec);
#else
    // Y2038-unsafe function - but should be ok for uptime
    // ref: https://sourceware.org/glibc/wiki/Y2038ProofnessDesign
    if (gettimeofday(&(PARSE->uptime), NULL))
        fatal_perror("Error - gettimeofday(): ");
    append_string  (PARSE, &pos, "uptime");
    append_token   (PARSE, &pos, '=');
    append_ulong   (PARSE, &pos, PARSE->uptime.tv_sec);
#endif

#if defined(HAVE_CLOCK_GETTIME)
    // Y2038-unsafe function -  FIXME
    // ref: https://sourceware.org/glibc/wiki/Y2038ProofnessDesign
    if (clock_gettime(CLOCK_REALTIME, &starttime))
        fatal_perror("Error - clock_gettime(): ");
    append_string  (PARSE, &pos, "starttime");
    append_token   (PARSE, &pos, '=');
    append_ulong   (PARSE, &pos, starttime.tv_sec);
#else
    // Y2038-unsafe function - FIXME
    // ref: https://sourceware.org/glibc/wiki/Y2038ProofnessDesign
    if (gettimeofday(&starttime, NULL))
        fatal_perror("Error - gettimeofday(): ");
    append_string  (PARSE, &pos, "starttime");
    append_token   (PARSE, &pos, '=');
    append_ulong   (PARSE, &pos, starttime.tv_sec);
#endif

    append_string  (PARSE, &pos, "inbufsize");
    append_token   (PARSE, &pos, '=');
    append_ulong   (PARSE, &pos, sizeof(inbuf_t));

    append_string  (PARSE, &pos, "inbufcapacity");
    append_token   (PARSE, &pos, '=');
    append_ulong   (PARSE, &pos, INBUFIZE);

    append_string  (PARSE, &pos, "elemsize");
    append_token   (PARSE, &pos, '=');
    append_ulong   (PARSE, &pos, sizeof(elem_t));

    append_string  (PARSE, &pos, "elemmallocsize");
    append_token   (PARSE, &pos, '=');
    append_ulong   (PARSE, &pos, LISTALLOCNUM * sizeof(elem_t));

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
    static char buf[STATS_BUF_SIZE];

    char *pos = &buf[0];  // NB non-static.  stats are updated and re-formatted on each call
    
    PARSE->sep = '\0';
#ifdef HAVE_CLOCK_GETTIME
    // Y2038-unsafe struct - but should be ok for uptime
    // ref: https://sourceware.org/glibc/wiki/Y2038ProofnessDesign
    struct timespec nowtime;
#else
    // Y2038-unsafe struct - but should be ok for uptime
    // ref: https://sourceware.org/glibc/wiki/Y2038ProofnessDesign
    struct timeval nowtime;
#endif
    long runtime;    // runtime in seconds

    append_string  (PARSE, &pos, "stats_running");
    append_token   (PARSE, &pos, '[');

#if defined(HAVE_CLOCK_GETTIME)
    // Y2038-unsafe struct - but should be ok for uptime
    // ref: https://sourceware.org/glibc/wiki/Y2038ProofnessDesign
    if (clock_gettime(CLOCK_BOOTTIME, &nowtime))
        fatal_perror("Error - clock_gettime(): ");
    runtime = ((uint64_t)nowtime.tv_sec * TEN9 + (uint64_t)nowtime.tv_nsec)
            - ((uint64_t)(PARSE->uptime.tv_sec) * TEN9 + (uint64_t)(PARSE->uptime.tv_nsec));
#else
    // Y2038-unsafe struct - but should be ok for uptime
    // ref: https://sourceware.org/glibc/wiki/Y2038ProofnessDesign
    if (gettimeofday(&nowtime, NULL))
        fatal_perror("Error - gettimeofday(): ");
    runtime = ((uint64_t)nowtime.tv_sec * TEN9 + (uint64_t)nowtime.tv_usec) * TEN3
            - ((uint64_t)(PARSE->uptime.tv_sec) * TEN9 + (uint64_t)(PARSE->uptime.tv_usec) * TEN3);
#endif

    append_string  (PARSE, &pos, "runtime");
    append_token   (PARSE, &pos, '=');
    append_runtime (PARSE, &pos, runtime/TEN9, runtime%TEN9);

    append_string  (PARSE, &pos, "files");
    append_token   (PARSE, &pos, '=');
    append_ulong   (PARSE, &pos, PARSE->TOKEN.stat_filecount);

    append_string  (PARSE, &pos, "lines");
    append_token   (PARSE, &pos, '=');
    append_ulong   (PARSE, &pos, 1 + (PARSE->TOKEN.stat_lfcount ? PARSE->TOKEN.stat_lfcount : PARSE->TOKEN.stat_crcount));

    append_string  (PARSE, &pos, "inacts");
    append_token   (PARSE, &pos, '=');
    append_ulong   (PARSE, &pos, PARSE->stat_inactcount);

    append_string  (PARSE, &pos, "inacts_per_second");
    append_token   (PARSE, &pos, '=');
    append_ulong   (PARSE, &pos, PARSE->stat_inactcount*TEN9/runtime);

    append_string  (PARSE, &pos, "sameas");
    append_token   (PARSE, &pos, '=');
    append_ulong   (PARSE, &pos, PARSE->stat_sameas);

    append_string  (PARSE, &pos, "patternacts");
    append_token   (PARSE, &pos, '=');
    append_ulong   (PARSE, &pos, PARSE->stat_patternactcount);

    append_string  (PARSE, &pos, "nonpatternacts");
    append_token   (PARSE, &pos, '=');
    append_ulong   (PARSE, &pos, PARSE->stat_nonpatternactcount);

    append_string  (PARSE, &pos, "patternmatches");
    append_token   (PARSE, &pos, '=');
    append_ulong   (PARSE, &pos, PARSE->stat_patternmatches);

    append_string  (PARSE, &pos, "outacts");
    append_token   (PARSE, &pos, '=');
    append_ulong   (PARSE, &pos, PARSE->stat_outactcount);

    append_string  (PARSE, &pos, "strings");
    append_token   (PARSE, &pos, '=');
    append_ulong   (PARSE, &pos, PARSE->TOKEN.stat_stringcount);

    append_string  (PARSE, &pos, "fragmentss");
    append_token   (PARSE, &pos, '=');
    append_ulong   (PARSE, &pos, PARSE->TOKEN.stat_fragcount);

    append_string  (PARSE, &pos, "inchars");
    append_token   (PARSE, &pos, '=');
    append_ulong   (PARSE, &pos, PARSE->TOKEN.stat_inchars);

    append_string  (PARSE, &pos, "chars_per_second");
    append_token   (PARSE, &pos, '=');
    append_ulong   (PARSE, &pos, PARSE->TOKEN.stat_inchars + TEN9 / runtime);

    append_string  (PARSE, &pos, "inbufmax");
    append_token   (PARSE, &pos, '=');
    append_ulong   (PARSE, &pos, PARSE->TOKEN.LIST.INBUF.stat_inbufmax);

    append_string  (PARSE, &pos, "inbufnow");
    append_token   (PARSE, &pos, '=');
    append_ulong   (PARSE, &pos, PARSE->TOKEN.LIST.INBUF.stat_inbufnow);

    append_string  (PARSE, &pos, "elemmax");
    append_token   (PARSE, &pos, '=');
    append_ulong   (PARSE, &pos, PARSE->TOKEN.LIST.stat_elemmax);

    append_string  (PARSE, &pos, "elemnow");
    append_token   (PARSE, &pos, '=');
    append_ulong   (PARSE, &pos, PARSE->TOKEN.LIST.stat_elemnow);

    append_string  (PARSE, &pos, "inbufmallocsize");
    append_token   (PARSE, &pos, '=');
    append_ulong   (PARSE, &pos, INBUFALLOCNUM * sizeof(inbuf_t));

    append_string  (PARSE, &pos, "inbufmalloccount");
    append_token   (PARSE, &pos, '=');
    append_ulong   (PARSE, &pos, PARSE->TOKEN.LIST.INBUF.stat_inbufmalloc);

    append_string  (PARSE, &pos, "inbufmalloctotal");
    append_token   (PARSE, &pos, '=');
    append_ulong   (PARSE, &pos, PARSE->TOKEN.LIST.INBUF.stat_inbufmalloc * INBUFALLOCNUM * sizeof(inbuf_t));

    append_string  (PARSE, &pos, "elemmalloccount");
    append_token   (PARSE, &pos, '=');
    append_ulong   (PARSE, &pos, PARSE->TOKEN.LIST.stat_elemmalloc);

    append_string  (PARSE, &pos, "elemmalloctotal");
    append_token   (PARSE, &pos, '=');
    append_ulong   (PARSE, &pos, PARSE->TOKEN.LIST.stat_elemmalloc * LISTALLOCNUM * sizeof(elem_t));

    append_string  (PARSE, &pos, "malloctotal");
    append_token   (PARSE, &pos, '=');
    append_ulong   (PARSE, &pos, (PARSE->TOKEN.LIST.stat_elemmalloc * LISTALLOCNUM * sizeof(elem_t))
                        + (PARSE->TOKEN.LIST.INBUF.stat_inbufmalloc * INBUFALLOCNUM * sizeof(inbuf_t)));

    append_token   (PARSE, &pos, ']');

    assert(pos < buf+STATS_BUF_SIZE);

    return buf;
}
