/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef PROCESS_H
#define PROCESS_H

#ifdef __cplusplus
extern "C" {
#endif

#define TEN9 1000000000
#define TEN3 1000

// Graph Object Model
typedef struct {
    elem_t *nodes;    // tree of nodes (sorted, unique)
    elem_t *edges;    // tree of edges (sorted, unique)
} GOM_t;

struct process_s {
    THREAD_t *THREAD;          // THREADs in this PROCESS

// FIXME - replace with a properly formed QRY
    char needstats;            // flag set if -s on command line
//

    // info collected by session();
    char *progname;
    char *username;
    char *hostname;          
    char *osname;
    char *osrelease;
    char *osmachine;

    GOM_t MUM;       // Primary, in-memory graph  (one of the sisters, never explicitly named)
    GOM_t SALLY;     // Aunt Sally,    Statistics  
    GOM_t GUDRUN;    // Aunt Gudrun,   Grammar  (devine knowledge)

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
