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

// This code collects info from the environment to:
//          - provide a readable unique session name
//          - capture start time for stats.
//
// This code is very likely Linux specific.  It is 
// here so that it can be conditionally adapted as needed.

void session(char *progname)
{
    int len;
	struct timespec starttime;
    struct passwd *pw;
    uid_t uid;
    pid_t pid;
    struct utsname unamebuf;

    len = 0;

    // command name

    len += strlen(progname) +1;             // e.g.  "g_"

    // user info
    uid = geteuid();
    pw = getpwuid(uid);
    if (!pw) {
        fprintf(stderr,"%s: Error: Cannot find username for UID %u\n", progname, (unsigned)uid);
        exit(EXIT_FAILURE);
    }
    len += strlen(pw->pw_name) +1;          // e.g.  "ellson@"

    // machine info
    if (uname(&unamebuf) != 0) {
        // FIXME - use errno
        fprintf(stderr,"%s: Error: Cannot find machine name\n", progname);
        exit(EXIT_FAILURE);
    } 
    len += strlen(unamebuf.nodename) +1;    // e.g. "work_" 

    // process info
    pid = getpid();
    len += 10;                              // e.g. "123_"
                                            // FIXME - how much do I really need?

    // starttime
	if (clock_gettime(CLOCK_MONOTONIC_RAW, &starttime) != 0) {
        // FIXME - use errno
        fprintf(stderr,"%s: Error: Cannot determine start time from clock_gettime()", progname);
        exit(EXIT_FAILURE);
    }
    len += 10;                              // e.g. 123456789.
}
