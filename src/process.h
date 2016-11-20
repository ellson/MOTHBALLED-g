/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef PROCESS_H
#define PROCESS_H

#ifdef __cplusplus
extern "C" {
#endif

#define TEN9 1000000000
#define TEN3 1000

struct process_s {
    THREAD_t *THREAD;          // THREADs in this PROCESS

// FIXME - replace with a properly formed QRY
    char needstats;            // flag set if -s on command line
//

    // infor collected by session();
    char *progname;
    char *username;
    char *hostname;          
    char *osname;
    char *osrelease;
    char *osmachine;
    uint64_t pid;
    uint64_t uptime;
    uint64_t uptime_nsec;
    uint64_t starttime;
    uint64_t starttime_nsec;
};

#ifdef __cplusplus
}
#endif

#endif
