/* vim:set shiftwidth=4 ts=8 expandtab: */

 struct context_s {            // input_context
    char *progname;            // name of program
    int *pargc;                // remaining filenames from command line
    char **argv;
    char *filename;            // name of file currently being processed, or "-" for stdin
    FILE *file;                // open file handle for file currently being processed
    FILE *out;                 // typically stdout for parser debug outputs
    inbuf_t *inbuf;            // the active input buffer
    unsigned char *in;         // next charater to be processed
    char *username;            // set by first call to g_session
    char *hostname;            // ditto
    char *tempdir;             // temporary dir for container files
    inbuf_t *free_inbuf_list;  // linked list of unused inbufs
    elem_t *free_elem_list;    // linked list of unused list elems
    state_t insi;              // state represented by last character read
    state_t ei;                // ei, bi are used to determine whitespace needs around STRINGs
    state_t bi;
    state_t verb;              // the "verb" for the ACT. Default is "add",
                                                    // '~' is "delete",
                                                    // '?' is "query"
    state_t state;             // last state entered
    char has_ast;              // flag set if an '*' is found in a STRING
    char has_cousin;           // flag set if a COUSIN is found in any EDGE of the ACT (forward EDGE to ancestors for processing)
    char in_quote;             // flag set if between "..."
    char has_quote;            // flag set if STRING contains one or more DQT fragments
    char needstats;            // flag set if -s on command line
    char sep;                  // the next separator (either 0, or ' ' if following a STRING that requires a separator.
                               //   may be ignored if the next character is a token which implicitly separates.)
    style_t style;             // spacing style in emitted outputs
                               //   may be ignored if the next character is a token which implicitly separates.)
    int containment;           // depth of containment
    char template[32];         // place to keep template for mkdtemp()
    long linecount_at_start;   // activity line count when this file was opened.
    elem_t myname;             // header for a list of components of my name (in the same form as subjects)
    elem_t *hash_buckets[64];  // 64 buckets of name hashes and FILE*.
    ikea_box_t *namehash_buckets[64];  // 64 buckets of namehashes
    long stat_filecount;       // various stats
    long stat_lfcount;
    long stat_crcount;
    long stat_inchars;
    long stat_actcount;
    long stat_sameas;
    long stat_patterncount;
    long stat_patternmatches;
    long stat_containercount;
    long stat_stringcount;
    long stat_fragcount;
    long stat_inbufmalloc;
    long stat_inbufmax;
    long stat_inbufnow;
    long stat_elemmalloc;
    long stat_elemmax;
    long stat_elemnow;

#if defined(HAVE_CLOCK_GETTIME)
    // Y2038-unsafe struct - but should be ok for uptime
    // ref: https://sourceware.org/glibc/wiki/Y2038ProofnessDesign
    struct timespec uptime;     // time with subsec resolution since boot, used as the base for runtime calculations
#else
    // Y2038-unsafe struct - but should be ok for uptime
    // ref: https://sourceware.org/glibc/wiki/Y2038ProofnessDesign
    struct timeval uptime;      // time with subsec resolution since boot, used as the base for runtime calculations
#endif
    pid_t pid;
};

struct container_context_s {    // container_context
    context_t *context;         // the input context
    elem_t subject;             // Preceeding ACT's subject, until this ACT's SUBJECT has been parsed
                                // and processd by sameas()  - at which point it becomes this ACT's subject.
                                // (So: in SUBJECT parsing it is the previous ACT's subject and used for sameas()
                                // substitutions once a new SUBJECT has been parsed. For ATTRIBUTES
                                // and CONTAINERS it is this ACT.   It is the basis of the name for
                                // the output files for contents.)
    char is_pattern;            // flag set if '*' occurred in SUBJECT
    state_t subject_type;       // set by sameas() to record if the SUBJECT is NODE(s), or EDGE(s),
                                //   and to check that it is not a mix of NODE(s) and EDGE(s).
    elem_t node_pattern_acts;   // complete ACTs from whenever the NODE subject contains an "*"
    elem_t edge_pattern_acts;   // complete ACTs from whenever the EDGE subject contains an "*"
    FILE *out;                  // the output file for this container

    // FIXME  - place for fork header for layout process...

};
