#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <pwd.h>
#include <assert.h>

#include "grammar.h"
#include "inbuf.h"
#include "list.h"
#include "context.h"
#include "emit.h"
#include "info.h"

// globals
struct timespec uptime;
long stat_filecount;
long stat_lfcount;
long stat_crcount;
long stat_inchars;
long stat_actcount;
long stat_patterncount;
long stat_containercount;
long stat_stringcount;
long stat_fragcount;
long stat_inbufmalloc;
long stat_inbufmax;
long stat_inbufnow;
long stat_elemmalloc;
long stat_elemmax;
long stat_elemnow;

// This code collects info from the environment to:
//          - populate session info into attributes of a 'g' NODE
//          - provide a readable unique session name for session-freeze tar file names
//          - capture start time for stats.

// This code is very likely Linux specific, and may need canoditionals for porting to other OS.

// The resulting info is collected into a buffer using minimal spacing g format
// e.g.      "session[progname=g username=ellson hostname=work .... ]

// There is an attribute pretty-printer function for when this is printed for the user.

// This info is gathered just once, so a statically sized buffer is used. It is only filled
// on the first call,  if session() is called again the same result is used.


#define SESSION_BUF_SIZE 1024

char * g_session(char *progname)
{
    static char buf[SESSION_BUF_SIZE];
    static char *pos = &buf[0];  // NB. static. This initalization happens only once

	struct timespec starttime;
    struct passwd *pw;
    uid_t uid;
    pid_t pid;
    struct utsname unamebuf;
    char sep;

    if (pos != &buf[0]) { // have we been here before?
        return buf;
    }

    sep = 0;

    g_append_string  (&pos, &sep, "session");
    g_append_token   (&pos, &sep, '[');

    g_append_string  (&pos, &sep, "progname");
    g_append_token   (&pos, &sep, '=');
    g_append_qstring (&pos, &sep, progname);

    pid = getpid();
    g_append_string  (&pos, &sep, "pid");
    g_append_token   (&pos, &sep, '=');
    g_append_ulong   (&pos, &sep, pid);

    uid = geteuid();
    pw = getpwuid(uid);
    if (!pw) {
        fprintf(stderr,"%s: Error: Cannot find username for UID %u\n", progname, (unsigned)uid);
        exit(EXIT_FAILURE);
    }
    g_append_string  (&pos, &sep, "username");
    g_append_token   (&pos, &sep, '=');
    g_append_string  (&pos, &sep, pw->pw_name);

    if (uname(&unamebuf) != 0) {
        // FIXME - use errno
        fprintf(stderr,"%s: Error: Cannot find machine name\n", progname);
        exit(EXIT_FAILURE);
    } 
    g_append_string  (&pos, &sep, "hostname");
    g_append_token   (&pos, &sep, '=');
    g_append_string  (&pos, &sep, unamebuf.nodename);

	if (clock_gettime(CLOCK_BOOTTIME, &uptime) != 0) {
        // FIXME - use errno
        fprintf(stderr,"%s: Error: Cannot determine uptime from clock_gettime()", progname);
        exit(EXIT_FAILURE);
    }
    g_append_string  (&pos, &sep, "uptime");
    g_append_token   (&pos, &sep, '=');
    g_append_ulong   (&pos, &sep, uptime.tv_sec);

	if (clock_gettime(CLOCK_REALTIME, &starttime) != 0) {
        // FIXME - use errno
        fprintf(stderr,"%s: Error: Cannot determine starttime from clock_gettime()", progname);
        exit(EXIT_FAILURE);
    }
    g_append_string  (&pos, &sep, "starttime");
    g_append_token   (&pos, &sep, '=');
    g_append_ulong   (&pos, &sep, starttime.tv_sec);

    g_append_token   (&pos, &sep, ']');
    return buf;
}

#define STATS_BUF_SIZE 2048
#define TEN9 1000000000

char * g_stats(char * progname)
{
    static char buf[STATS_BUF_SIZE];

    char *pos = &buf[0];  // NB non-static.  stats are updated and re-formatted on each call
	struct timespec nowtime;
	long runtime;    // runtime in nano-seconds
    char sep;

    sep = 0;

    g_append_string  (&pos, &sep, "stats");
    g_append_token   (&pos, &sep, '[');

	if (clock_gettime(CLOCK_BOOTTIME, &nowtime) != 0) {
        // FIXME - use errno
        fprintf(stderr,"%s: Error: Cannot determine runtime from clock_gettime()", progname);
        exit(EXIT_FAILURE);
    }
	runtime = ((unsigned long)nowtime.tv_sec * TEN9 + (unsigned long)nowtime.tv_nsec)
            - ((unsigned long)uptime.tv_sec * TEN9 + (unsigned long)uptime.tv_nsec);

    g_append_string  (&pos, &sep, "runtime");
    g_append_token   (&pos, &sep, '=');
    g_append_runtime (&pos, &sep, runtime/TEN9, runtime%TEN9);

    g_append_string  (&pos, &sep, "files");
    g_append_token   (&pos, &sep, '=');
    g_append_ulong   (&pos, &sep, stat_filecount);

    g_append_string  (&pos, &sep, "lines");
    g_append_token   (&pos, &sep, '=');
    g_append_ulong   (&pos, &sep, 1 + (stat_lfcount ? stat_lfcount : stat_crcount));

    g_append_string  (&pos, &sep, "acts");
    g_append_token   (&pos, &sep, '=');
    g_append_ulong   (&pos, &sep, stat_actcount);

    g_append_string  (&pos, &sep, "acts_per_second");
    g_append_token   (&pos, &sep, '=');
    g_append_ulong   (&pos, &sep, stat_actcount*TEN9/runtime);

    g_append_string  (&pos, &sep, "patterns");
    g_append_token   (&pos, &sep, '=');
    g_append_ulong   (&pos, &sep, stat_patterncount);

    g_append_string  (&pos, &sep, "containers");
    g_append_token   (&pos, &sep, '=');
    g_append_ulong   (&pos, &sep, stat_containercount);

    g_append_string  (&pos, &sep, "strings");
    g_append_token   (&pos, &sep, '=');
    g_append_ulong   (&pos, &sep, stat_stringcount);

    g_append_string  (&pos, &sep, "fragmentss");
    g_append_token   (&pos, &sep, '=');
    g_append_ulong   (&pos, &sep, stat_fragcount);

    g_append_string  (&pos, &sep, "inchars");
    g_append_token   (&pos, &sep, '=');
    g_append_ulong   (&pos, &sep, stat_inchars);

    g_append_string  (&pos, &sep, "chars_per_second");
    g_append_token   (&pos, &sep, '=');
    g_append_ulong   (&pos, &sep, stat_inchars + TEN9 / runtime);

    g_append_string  (&pos, &sep, "inbufsize");
    g_append_token   (&pos, &sep, '=');
    g_append_ulong   (&pos, &sep, sizeof(inbuf_t));

    g_append_string  (&pos, &sep, "inbufmax");
    g_append_token   (&pos, &sep, '=');
    g_append_ulong   (&pos, &sep, stat_inbufmax);

    g_append_string  (&pos, &sep, "inbufnow");
    g_append_token   (&pos, &sep, '=');
    g_append_ulong   (&pos, &sep, stat_inbufnow);

    g_append_string  (&pos, &sep, "elemsize");
    g_append_token   (&pos, &sep, '=');
    g_append_ulong   (&pos, &sep, size_elem_t);

    g_append_string  (&pos, &sep, "elemmax");
    g_append_token   (&pos, &sep, '=');
    g_append_ulong   (&pos, &sep, stat_elemmax);

    g_append_string  (&pos, &sep, "elemnow");
    g_append_token   (&pos, &sep, '=');
    g_append_ulong   (&pos, &sep, stat_elemnow);

    g_append_string  (&pos, &sep, "inbufmallocsize");
    g_append_token   (&pos, &sep, '=');
    g_append_ulong   (&pos, &sep, INBUFALLOCNUM * sizeof(inbuf_t));

    g_append_string  (&pos, &sep, "inbufmalloccount");
    g_append_token   (&pos, &sep, '=');
    g_append_ulong   (&pos, &sep, stat_inbufmalloc);

    g_append_string  (&pos, &sep, "inbufmalloctotal");
    g_append_token   (&pos, &sep, '=');
    g_append_ulong   (&pos, &sep, stat_inbufmalloc * INBUFALLOCNUM * sizeof(inbuf_t));

    g_append_string  (&pos, &sep, "elemmallocsize");
    g_append_token   (&pos, &sep, '=');
    g_append_ulong   (&pos, &sep, LISTALLOCNUM * size_elem_t);

    g_append_string  (&pos, &sep, "elemmalloccount");
    g_append_token   (&pos, &sep, '=');
    g_append_ulong   (&pos, &sep, stat_elemmalloc);

    g_append_string  (&pos, &sep, "elemmalloctotal");
    g_append_token   (&pos, &sep, '=');
    g_append_ulong   (&pos, &sep, stat_elemmalloc * LISTALLOCNUM * size_elem_t);

    g_append_string  (&pos, &sep, "malloctotal");
    g_append_token   (&pos, &sep, '=');
    g_append_ulong   (&pos, &sep, (stat_elemmalloc * LISTALLOCNUM * size_elem_t)
                        + (stat_inbufmalloc * INBUFALLOCNUM * sizeof(inbuf_t)));

    g_append_token   (&pos, &sep, ']');
    return buf;
}
