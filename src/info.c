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
 * @param C context
 * @return formatted string result
 */
char * je_session(CONTEXT_t *C)
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

    je_append_string  (C, &pos, "stats_fixed");
    je_append_token   (C, &pos, '[');

    je_append_string  (C, &pos, "progname");
    je_append_token   (C, &pos, '=');
    je_append_string  (C, &pos, C->progname);

    C->pid = getpid();
    je_append_string  (C, &pos, "pid");
    je_append_token   (C, &pos, '=');
    je_append_ulong   (C, &pos, C->pid);

    uid = geteuid();
    if (!(pw = getpwuid(uid)))
        fatal_perror("Error - getpwuid(): ");
    C->username = pw->pw_name;
    je_append_string  (C, &pos, "username");
    je_append_token   (C, &pos, '=');
    je_append_string  (C, &pos, C->username);

    if (uname(&unamebuf))
        fatal_perror("Error - uname(): ");
    C->hostname = unamebuf.nodename;
    je_append_string  (C, &pos, "hostname");
    je_append_token   (C, &pos, '=');
    je_append_string  (C, &pos, C->hostname);

#if defined(HAVE_CLOCK_GETTIME)
    // Y2038-unsafe function - but should be ok for uptime
    // ref: https://sourceware.org/glibc/wiki/Y2038ProofnessDesign
    if (clock_gettime(CLOCK_BOOTTIME, &(C->uptime)))
        fatal_perror("Error - clock_gettime(): ");
    je_append_string  (C, &pos, "uptime");
    je_append_token   (C, &pos, '=');
    je_append_ulong   (C, &pos, C->uptime.tv_sec);
#else
    // Y2038-unsafe function - but should be ok for uptime
    // ref: https://sourceware.org/glibc/wiki/Y2038ProofnessDesign
    if (gettimeofday(&(C->uptime), NULL))
        fatal_perror("Error - gettimeofday(): ");
    je_append_string  (C, &pos, "uptime");
    je_append_token   (C, &pos, '=');
    je_append_ulong   (C, &pos, C->uptime.tv_sec);
#endif

#if defined(HAVE_CLOCK_GETTIME)
    // Y2038-unsafe function -  FIXME
    // ref: https://sourceware.org/glibc/wiki/Y2038ProofnessDesign
    if (clock_gettime(CLOCK_REALTIME, &starttime))
        fatal_perror("Error - clock_gettime(): ");
    je_append_string  (C, &pos, "starttime");
    je_append_token   (C, &pos, '=');
    je_append_ulong   (C, &pos, starttime.tv_sec);
#else
    // Y2038-unsafe function - FIXME
    // ref: https://sourceware.org/glibc/wiki/Y2038ProofnessDesign
    if (gettimeofday(&starttime, NULL))
        fatal_perror("Error - gettimeofday(): ");
    je_append_string  (C, &pos, "starttime");
    je_append_token   (C, &pos, '=');
    je_append_ulong   (C, &pos, starttime.tv_sec);
#endif

    je_append_string  (C, &pos, "inbufsize");
    je_append_token   (C, &pos, '=');
    je_append_ulong   (C, &pos, sizeof(inbuf_t));

    je_append_string  (C, &pos, "inbufcapacity");
    je_append_token   (C, &pos, '=');
    je_append_ulong   (C, &pos, INBUFIZE);

    je_append_string  (C, &pos, "elemsize");
    je_append_token   (C, &pos, '=');
    je_append_ulong   (C, &pos, sizeof(elem_t));

    je_append_string  (C, &pos, "elemmallocsize");
    je_append_token   (C, &pos, '=');
    je_append_ulong   (C, &pos, LISTALLOCNUM * sizeof(elem_t));

    je_append_token   (C, &pos, ']');

    assert(pos < buf+SESSION_BUF_SIZE);

    return buf;
}

#define STATS_BUF_SIZE 2048
#define TEN9 1000000000
#define TEN3 1000

/**
 * format running stats as a string
 *
 * @param C context
 * @return formatted string
 */
char * je_stats(CONTEXT_t *C)
{
    static char buf[STATS_BUF_SIZE];

    char *pos = &buf[0];  // NB non-static.  stats are updated and re-formatted on each call
    
    C->sep = '\0';
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

    je_append_string  (C, &pos, "stats_running");
    je_append_token   (C, &pos, '[');

#if defined(HAVE_CLOCK_GETTIME)
    // Y2038-unsafe struct - but should be ok for uptime
    // ref: https://sourceware.org/glibc/wiki/Y2038ProofnessDesign
    if (clock_gettime(CLOCK_BOOTTIME, &nowtime))
        fatal_perror("Error - clock_gettime(): ");
    runtime = ((uint64_t)nowtime.tv_sec * TEN9 + (uint64_t)nowtime.tv_nsec)
            - ((uint64_t)(C->uptime.tv_sec) * TEN9 + (uint64_t)(C->uptime.tv_nsec));
#else
    // Y2038-unsafe struct - but should be ok for uptime
    // ref: https://sourceware.org/glibc/wiki/Y2038ProofnessDesign
    if (gettimeofday(&nowtime, NULL))
        fatal_perror("Error - gettimeofday(): ");
    runtime = ((uint64_t)nowtime.tv_sec * TEN9 + (uint64_t)nowtime.tv_usec) * TEN3
            - ((uint64_t)(C->uptime.tv_sec) * TEN9 + (uint64_t)(C->uptime.tv_usec) * TEN3);
#endif

    je_append_string  (C, &pos, "runtime");
    je_append_token   (C, &pos, '=');
    je_append_runtime (C, &pos, runtime/TEN9, runtime%TEN9);

    je_append_string  (C, &pos, "files");
    je_append_token   (C, &pos, '=');
    je_append_ulong   (C, &pos, C->TOKEN.stat_filecount);

    je_append_string  (C, &pos, "lines");
    je_append_token   (C, &pos, '=');
    je_append_ulong   (C, &pos, 1 + (C->TOKEN.stat_lfcount ? C->TOKEN.stat_lfcount : C->TOKEN.stat_crcount));

    je_append_string  (C, &pos, "inacts");
    je_append_token   (C, &pos, '=');
    je_append_ulong   (C, &pos, C->stat_inactcount);

    je_append_string  (C, &pos, "inacts_per_second");
    je_append_token   (C, &pos, '=');
    je_append_ulong   (C, &pos, C->stat_inactcount*TEN9/runtime);

    je_append_string  (C, &pos, "sameas");
    je_append_token   (C, &pos, '=');
    je_append_ulong   (C, &pos, C->stat_sameas);

    je_append_string  (C, &pos, "patternacts");
    je_append_token   (C, &pos, '=');
    je_append_ulong   (C, &pos, C->stat_patternactcount);

    je_append_string  (C, &pos, "nonpatternacts");
    je_append_token   (C, &pos, '=');
    je_append_ulong   (C, &pos, C->stat_nonpatternactcount);

    je_append_string  (C, &pos, "patternmatches");
    je_append_token   (C, &pos, '=');
    je_append_ulong   (C, &pos, C->stat_patternmatches);

    je_append_string  (C, &pos, "outacts");
    je_append_token   (C, &pos, '=');
    je_append_ulong   (C, &pos, C->stat_outactcount);

    je_append_string  (C, &pos, "strings");
    je_append_token   (C, &pos, '=');
    je_append_ulong   (C, &pos, C->TOKEN.stat_stringcount);

    je_append_string  (C, &pos, "fragmentss");
    je_append_token   (C, &pos, '=');
    je_append_ulong   (C, &pos, C->TOKEN.stat_fragcount);

    je_append_string  (C, &pos, "inchars");
    je_append_token   (C, &pos, '=');
    je_append_ulong   (C, &pos, C->TOKEN.stat_inchars);

    je_append_string  (C, &pos, "chars_per_second");
    je_append_token   (C, &pos, '=');
    je_append_ulong   (C, &pos, C->TOKEN.stat_inchars + TEN9 / runtime);

    je_append_string  (C, &pos, "inbufmax");
    je_append_token   (C, &pos, '=');
    je_append_ulong   (C, &pos, C->TOKEN.LIST.INBUF.stat_inbufmax);

    je_append_string  (C, &pos, "inbufnow");
    je_append_token   (C, &pos, '=');
    je_append_ulong   (C, &pos, C->TOKEN.LIST.INBUF.stat_inbufnow);

    je_append_string  (C, &pos, "elemmax");
    je_append_token   (C, &pos, '=');
    je_append_ulong   (C, &pos, C->TOKEN.LIST.stat_elemmax);

    je_append_string  (C, &pos, "elemnow");
    je_append_token   (C, &pos, '=');
    je_append_ulong   (C, &pos, C->TOKEN.LIST.stat_elemnow);

    je_append_string  (C, &pos, "inbufmallocsize");
    je_append_token   (C, &pos, '=');
    je_append_ulong   (C, &pos, INBUFALLOCNUM * sizeof(inbuf_t));

    je_append_string  (C, &pos, "inbufmalloccount");
    je_append_token   (C, &pos, '=');
    je_append_ulong   (C, &pos, C->TOKEN.LIST.INBUF.stat_inbufmalloc);

    je_append_string  (C, &pos, "inbufmalloctotal");
    je_append_token   (C, &pos, '=');
    je_append_ulong   (C, &pos, C->TOKEN.LIST.INBUF.stat_inbufmalloc * INBUFALLOCNUM * sizeof(inbuf_t));

    je_append_string  (C, &pos, "elemmalloccount");
    je_append_token   (C, &pos, '=');
    je_append_ulong   (C, &pos, C->TOKEN.LIST.stat_elemmalloc);

    je_append_string  (C, &pos, "elemmalloctotal");
    je_append_token   (C, &pos, '=');
    je_append_ulong   (C, &pos, C->TOKEN.LIST.stat_elemmalloc * LISTALLOCNUM * sizeof(elem_t));

    je_append_string  (C, &pos, "malloctotal");
    je_append_token   (C, &pos, '=');
    je_append_ulong   (C, &pos, (C->TOKEN.LIST.stat_elemmalloc * LISTALLOCNUM * sizeof(elem_t))
                        + (C->TOKEN.LIST.INBUF.stat_inbufmalloc * INBUFALLOCNUM * sizeof(inbuf_t)));

    je_append_token   (C, &pos, ']');

    assert(pos < buf+STATS_BUF_SIZE);

    return buf;
}
