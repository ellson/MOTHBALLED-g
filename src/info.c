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

char * session(char *progname)
{
    static char buf[SESSION_BUF_SIZE];
    static int space = SESSION_BUF_SIZE;

	struct timespec starttime;
    struct passwd *pw;
    uid_t uid;
    pid_t pid;
    struct utsname unamebuf;

    int len, pos;

    if (*buf != '\0') { // have we been here before?
        return buf;
    }

    len = 0;
    pos = 0;

    len = strcpy(buf+pos,"session["); 

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
