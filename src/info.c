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

char * g_session(container_context_t *CC)
{
    static char buf[SESSION_BUF_SIZE];
    static char *pos = &buf[0];  // NB. static. This initalization happens only once
    static struct passwd *pw;
    static struct utsname unamebuf;
	static struct timespec starttime;
    static uid_t uid;
    static pid_t pid;
    context_t *C = CC->context;

    if (pos != &buf[0]) { // have we been here before?
        return buf;
    }

    g_append_string  (CC, &pos, "session");
    g_append_token   (CC, &pos, '[');

    g_append_string  (CC, &pos, "progname");
    g_append_token   (CC, &pos, '=');
    g_append_string  (CC, &pos, C->progname);

    pid = getpid();
    g_append_string  (CC, &pos, "pid");
    g_append_token   (CC, &pos, '=');
    g_append_ulong   (CC, &pos, pid);

    uid = geteuid();
    pw = getpwuid(uid);
    if (!pw) {
        perror("Error - getpwuid(): ");
        exit(EXIT_FAILURE);
    }
    C->username = pw->pw_name;
    g_append_string  (CC, &pos, "username");
    g_append_token   (CC, &pos, '=');
    g_append_string  (CC, &pos, C->username);

    if (uname(&unamebuf) != 0) {
        perror("Error - uname(): ");
        exit(EXIT_FAILURE);
    } 
    C->hostname = unamebuf.nodename;
    g_append_string  (CC, &pos, "hostname");
    g_append_token   (CC, &pos, '=');
    g_append_string  (CC, &pos, C->hostname);

	if (clock_gettime(CLOCK_BOOTTIME, &(C->uptime)) != 0) {
        perror("Errror - clock_gettime(): ");
        exit(EXIT_FAILURE);
    }
    g_append_string  (CC, &pos, "uptime");
    g_append_token   (CC, &pos, '=');
    g_append_ulong   (CC, &pos, C->uptime.tv_sec);

	if (clock_gettime(CLOCK_REALTIME, &starttime) != 0) {
        perror("Errror - clock_gettime(): ");
        exit(EXIT_FAILURE);
    }
    g_append_string  (CC, &pos, "starttime");
    g_append_token   (CC, &pos, '=');
    g_append_ulong   (CC, &pos, starttime.tv_sec);

    g_append_token   (CC, &pos, ']');
    return buf;
}

#define STATS_BUF_SIZE 2048
#define TEN9 1000000000

char * g_stats(container_context_t *CC)
{
    static char buf[STATS_BUF_SIZE];

    char *pos = &buf[0];  // NB non-static.  stats are updated and re-formatted on each call
	struct timespec nowtime;
	long runtime;    // runtime in nano-seconds
    context_t *C = CC->context;

    g_append_string  (CC, &pos, "stats");
    g_append_token   (CC, &pos, '[');

	if (clock_gettime(CLOCK_BOOTTIME, &nowtime) != 0) {
        perror("Errror - clock_gettime(): ");
        exit(EXIT_FAILURE);
    }
	runtime = ((unsigned long)nowtime.tv_sec * TEN9 + (unsigned long)nowtime.tv_nsec)
            - ((unsigned long)(C->uptime.tv_sec) * TEN9 + (unsigned long)(C->uptime.tv_nsec));

    g_append_string  (CC, &pos, "runtime");
    g_append_token   (CC, &pos, '=');
    g_append_runtime (CC, &pos, runtime/TEN9, runtime%TEN9);

    g_append_string  (CC, &pos, "files");
    g_append_token   (CC, &pos, '=');
    g_append_ulong   (CC, &pos, C->stat_filecount);

    g_append_string  (CC, &pos, "lines");
    g_append_token   (CC, &pos, '=');
    g_append_ulong   (CC, &pos, 1 + (C->stat_lfcount ? C->stat_lfcount : C->stat_crcount));

    g_append_string  (CC, &pos, "acts");
    g_append_token   (CC, &pos, '=');
    g_append_ulong   (CC, &pos, C->stat_actcount);

    g_append_string  (CC, &pos, "acts_per_second");
    g_append_token   (CC, &pos, '=');
    g_append_ulong   (CC, &pos, C->stat_actcount*TEN9/runtime);

    g_append_string  (CC, &pos, "patterns");
    g_append_token   (CC, &pos, '=');
    g_append_ulong   (CC, &pos, C->stat_patterncount);

    g_append_string  (CC, &pos, "containers");
    g_append_token   (CC, &pos, '=');
    g_append_ulong   (CC, &pos, C->stat_containercount);

    g_append_string  (CC, &pos, "strings");
    g_append_token   (CC, &pos, '=');
    g_append_ulong   (CC, &pos, C->stat_stringcount);

    g_append_string  (CC, &pos, "fragmentss");
    g_append_token   (CC, &pos, '=');
    g_append_ulong   (CC, &pos, C->stat_fragcount);

    g_append_string  (CC, &pos, "inchars");
    g_append_token   (CC, &pos, '=');
    g_append_ulong   (CC, &pos, C->stat_inchars);

    g_append_string  (CC, &pos, "chars_per_second");
    g_append_token   (CC, &pos, '=');
    g_append_ulong   (CC, &pos, C->stat_inchars + TEN9 / runtime);

    g_append_string  (CC, &pos, "inbufsize");
    g_append_token   (CC, &pos, '=');
    g_append_ulong   (CC, &pos, sizeof(inbuf_t));

    g_append_string  (CC, &pos, "inbufmax");
    g_append_token   (CC, &pos, '=');
    g_append_ulong   (CC, &pos, C->stat_inbufmax);

    g_append_string  (CC, &pos, "inbufnow");
    g_append_token   (CC, &pos, '=');
    g_append_ulong   (CC, &pos, C->stat_inbufnow);

    g_append_string  (CC, &pos, "elemsize");
    g_append_token   (CC, &pos, '=');
    g_append_ulong   (CC, &pos, size_elem_t);

    g_append_string  (CC, &pos, "elemmax");
    g_append_token   (CC, &pos, '=');
    g_append_ulong   (CC, &pos, C->stat_elemmax);

    g_append_string  (CC, &pos, "elemnow");
    g_append_token   (CC, &pos, '=');
    g_append_ulong   (CC, &pos, C->stat_elemnow);

    g_append_string  (CC, &pos, "inbufmallocsize");
    g_append_token   (CC, &pos, '=');
    g_append_ulong   (CC, &pos, INBUFALLOCNUM * sizeof(inbuf_t));

    g_append_string  (CC, &pos, "inbufmalloccount");
    g_append_token   (CC, &pos, '=');
    g_append_ulong   (CC, &pos, C->stat_inbufmalloc);

    g_append_string  (CC, &pos, "inbufmalloctotal");
    g_append_token   (CC, &pos, '=');
    g_append_ulong   (CC, &pos, C->stat_inbufmalloc * INBUFALLOCNUM * sizeof(inbuf_t));

    g_append_string  (CC, &pos, "elemmallocsize");
    g_append_token   (CC, &pos, '=');
    g_append_ulong   (CC, &pos, LISTALLOCNUM * size_elem_t);

    g_append_string  (CC, &pos, "elemmalloccount");
    g_append_token   (CC, &pos, '=');
    g_append_ulong   (CC, &pos, C->stat_elemmalloc);

    g_append_string  (CC, &pos, "elemmalloctotal");
    g_append_token   (CC, &pos, '=');
    g_append_ulong   (CC, &pos, C->stat_elemmalloc * LISTALLOCNUM * size_elem_t);

    g_append_string  (CC, &pos, "malloctotal");
    g_append_token   (CC, &pos, '=');
    g_append_ulong   (CC, &pos, (C->stat_elemmalloc * LISTALLOCNUM * size_elem_t)
                        + (C->stat_inbufmalloc * INBUFALLOCNUM * sizeof(inbuf_t)));

    g_append_token   (CC, &pos, ']');
    return buf;
}
