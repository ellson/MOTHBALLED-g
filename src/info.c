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
char * je_session(PARSE_t * PARSE)
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

    je_append_string  (PARSE, &pos, "stats_fixed");
    je_append_token   (PARSE, &pos, '[');

    je_append_string  (PARSE, &pos, "progname");
    je_append_token   (PARSE, &pos, '=');
    je_append_string  (PARSE, &pos, PARSE->progname);

    PARSE->pid = getpid();
    je_append_string  (PARSE, &pos, "pid");
    je_append_token   (PARSE, &pos, '=');
    je_append_ulong   (PARSE, &pos, PARSE->pid);

    uid = geteuid();
    if (!(pw = getpwuid(uid)))
        fatal_perror("Error - getpwuid(): ");
    PARSE->username = pw->pw_name;
    je_append_string  (PARSE, &pos, "username");
    je_append_token   (PARSE, &pos, '=');
    je_append_string  (PARSE, &pos, PARSE->username);

    if (uname(&unamebuf))
        fatal_perror("Error - uname(): ");
    PARSE->hostname = unamebuf.nodename;
    je_append_string  (PARSE, &pos, "hostname");
    je_append_token   (PARSE, &pos, '=');
    je_append_string  (PARSE, &pos, PARSE->hostname);

#if defined(HAVE_CLOCK_GETTIME)
    // Y2038-unsafe function - but should be ok for uptime
    // ref: https://sourceware.org/glibc/wiki/Y2038ProofnessDesign
    if (clock_gettime(CLOCK_BOOTTIME, &(PARSE->uptime)))
        fatal_perror("Error - clock_gettime(): ");
    je_append_string  (PARSE, &pos, "uptime");
    je_append_token   (PARSE, &pos, '=');
    je_append_ulong   (PARSE, &pos, PARSE->uptime.tv_sec);
#else
    // Y2038-unsafe function - but should be ok for uptime
    // ref: https://sourceware.org/glibc/wiki/Y2038ProofnessDesign
    if (gettimeofday(&(PARSE->uptime), NULL))
        fatal_perror("Error - gettimeofday(): ");
    je_append_string  (PARSE, &pos, "uptime");
    je_append_token   (PARSE, &pos, '=');
    je_append_ulong   (PARSE, &pos, PARSE->uptime.tv_sec);
#endif

#if defined(HAVE_CLOCK_GETTIME)
    // Y2038-unsafe function -  FIXME
    // ref: https://sourceware.org/glibc/wiki/Y2038ProofnessDesign
    if (clock_gettime(CLOCK_REALTIME, &starttime))
        fatal_perror("Error - clock_gettime(): ");
    je_append_string  (PARSE, &pos, "starttime");
    je_append_token   (PARSE, &pos, '=');
    je_append_ulong   (PARSE, &pos, starttime.tv_sec);
#else
    // Y2038-unsafe function - FIXME
    // ref: https://sourceware.org/glibc/wiki/Y2038ProofnessDesign
    if (gettimeofday(&starttime, NULL))
        fatal_perror("Error - gettimeofday(): ");
    je_append_string  (PARSE, &pos, "starttime");
    je_append_token   (PARSE, &pos, '=');
    je_append_ulong   (PARSE, &pos, starttime.tv_sec);
#endif

    je_append_string  (PARSE, &pos, "inbufsize");
    je_append_token   (PARSE, &pos, '=');
    je_append_ulong   (PARSE, &pos, sizeof(inbuf_t));

    je_append_string  (PARSE, &pos, "inbufcapacity");
    je_append_token   (PARSE, &pos, '=');
    je_append_ulong   (PARSE, &pos, INBUFIZE);

    je_append_string  (PARSE, &pos, "elemsize");
    je_append_token   (PARSE, &pos, '=');
    je_append_ulong   (PARSE, &pos, sizeof(elem_t));

    je_append_string  (PARSE, &pos, "elemmallocsize");
    je_append_token   (PARSE, &pos, '=');
    je_append_ulong   (PARSE, &pos, LISTALLOCNUM * sizeof(elem_t));

    je_append_token   (PARSE, &pos, ']');

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
char * je_stats(PARSE_t * PARSE)
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

    je_append_string  (PARSE, &pos, "stats_running");
    je_append_token   (PARSE, &pos, '[');

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

    je_append_string  (PARSE, &pos, "runtime");
    je_append_token   (PARSE, &pos, '=');
    je_append_runtime (PARSE, &pos, runtime/TEN9, runtime%TEN9);

    je_append_string  (PARSE, &pos, "files");
    je_append_token   (PARSE, &pos, '=');
    je_append_ulong   (PARSE, &pos, PARSE->TOKEN.stat_filecount);

    je_append_string  (PARSE, &pos, "lines");
    je_append_token   (PARSE, &pos, '=');
    je_append_ulong   (PARSE, &pos, 1 + (PARSE->TOKEN.stat_lfcount ? PARSE->TOKEN.stat_lfcount : PARSE->TOKEN.stat_crcount));

    je_append_string  (PARSE, &pos, "inacts");
    je_append_token   (PARSE, &pos, '=');
    je_append_ulong   (PARSE, &pos, PARSE->stat_inactcount);

    je_append_string  (PARSE, &pos, "inacts_per_second");
    je_append_token   (PARSE, &pos, '=');
    je_append_ulong   (PARSE, &pos, PARSE->stat_inactcount*TEN9/runtime);

    je_append_string  (PARSE, &pos, "sameas");
    je_append_token   (PARSE, &pos, '=');
    je_append_ulong   (PARSE, &pos, PARSE->stat_sameas);

    je_append_string  (PARSE, &pos, "patternacts");
    je_append_token   (PARSE, &pos, '=');
    je_append_ulong   (PARSE, &pos, PARSE->stat_patternactcount);

    je_append_string  (PARSE, &pos, "nonpatternacts");
    je_append_token   (PARSE, &pos, '=');
    je_append_ulong   (PARSE, &pos, PARSE->stat_nonpatternactcount);

    je_append_string  (PARSE, &pos, "patternmatches");
    je_append_token   (PARSE, &pos, '=');
    je_append_ulong   (PARSE, &pos, PARSE->stat_patternmatches);

    je_append_string  (PARSE, &pos, "outacts");
    je_append_token   (PARSE, &pos, '=');
    je_append_ulong   (PARSE, &pos, PARSE->stat_outactcount);

    je_append_string  (PARSE, &pos, "strings");
    je_append_token   (PARSE, &pos, '=');
    je_append_ulong   (PARSE, &pos, PARSE->TOKEN.stat_stringcount);

    je_append_string  (PARSE, &pos, "fragmentss");
    je_append_token   (PARSE, &pos, '=');
    je_append_ulong   (PARSE, &pos, PARSE->TOKEN.stat_fragcount);

    je_append_string  (PARSE, &pos, "inchars");
    je_append_token   (PARSE, &pos, '=');
    je_append_ulong   (PARSE, &pos, PARSE->TOKEN.stat_inchars);

    je_append_string  (PARSE, &pos, "chars_per_second");
    je_append_token   (PARSE, &pos, '=');
    je_append_ulong   (PARSE, &pos, PARSE->TOKEN.stat_inchars + TEN9 / runtime);

    je_append_string  (PARSE, &pos, "inbufmax");
    je_append_token   (PARSE, &pos, '=');
    je_append_ulong   (PARSE, &pos, PARSE->TOKEN.LIST.INBUF.stat_inbufmax);

    je_append_string  (PARSE, &pos, "inbufnow");
    je_append_token   (PARSE, &pos, '=');
    je_append_ulong   (PARSE, &pos, PARSE->TOKEN.LIST.INBUF.stat_inbufnow);

    je_append_string  (PARSE, &pos, "elemmax");
    je_append_token   (PARSE, &pos, '=');
    je_append_ulong   (PARSE, &pos, PARSE->TOKEN.LIST.stat_elemmax);

    je_append_string  (PARSE, &pos, "elemnow");
    je_append_token   (PARSE, &pos, '=');
    je_append_ulong   (PARSE, &pos, PARSE->TOKEN.LIST.stat_elemnow);

    je_append_string  (PARSE, &pos, "inbufmallocsize");
    je_append_token   (PARSE, &pos, '=');
    je_append_ulong   (PARSE, &pos, INBUFALLOCNUM * sizeof(inbuf_t));

    je_append_string  (PARSE, &pos, "inbufmalloccount");
    je_append_token   (PARSE, &pos, '=');
    je_append_ulong   (PARSE, &pos, PARSE->TOKEN.LIST.INBUF.stat_inbufmalloc);

    je_append_string  (PARSE, &pos, "inbufmalloctotal");
    je_append_token   (PARSE, &pos, '=');
    je_append_ulong   (PARSE, &pos, PARSE->TOKEN.LIST.INBUF.stat_inbufmalloc * INBUFALLOCNUM * sizeof(inbuf_t));

    je_append_string  (PARSE, &pos, "elemmalloccount");
    je_append_token   (PARSE, &pos, '=');
    je_append_ulong   (PARSE, &pos, PARSE->TOKEN.LIST.stat_elemmalloc);

    je_append_string  (PARSE, &pos, "elemmalloctotal");
    je_append_token   (PARSE, &pos, '=');
    je_append_ulong   (PARSE, &pos, PARSE->TOKEN.LIST.stat_elemmalloc * LISTALLOCNUM * sizeof(elem_t));

    je_append_string  (PARSE, &pos, "malloctotal");
    je_append_token   (PARSE, &pos, '=');
    je_append_ulong   (PARSE, &pos, (PARSE->TOKEN.LIST.stat_elemmalloc * LISTALLOCNUM * sizeof(elem_t))
                        + (PARSE->TOKEN.LIST.INBUF.stat_inbufmalloc * INBUFALLOCNUM * sizeof(inbuf_t)));

    je_append_token   (PARSE, &pos, ']');

    assert(pos < buf+STATS_BUF_SIZE);

    return buf;
}
