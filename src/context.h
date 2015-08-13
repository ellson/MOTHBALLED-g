typedef struct container_context_s container_context_t;

typedef struct {           // input_context
    int *pargc;            // remaining filenames from command line
    char **argv;
    char *filename;        // name of file currently being processed, or "-" for stdin
    FILE *file;            // open file handle for file currently being processed
    long linecount_at_start; // activity line count when this file was opened.
    inbuf_t *inbuf;        // the active input buffer
    unsigned char *in;     // next charater to be processed
    state_t insi;          // state represented by last character read
    state_t ei;            // ei, bi are used to determine whitespace needs around STRINGs
    state_t bi;          
    state_t subj_type;     // used to verify homogenous SUBJECT
    char ast_seen;         // flag set if an '*' is found in a STRING
    char is_pattern;       // flag set if the '*' occurred in SUBJECT
    int containment;       // depth of containment
    FILE *out;             // the output file 
    FILE *err;             // the output file for errors
} context_t;

struct container_context_s { // container_context (also output context)
    elem_t prev_subject;   // preceeding ACT's subject,  for sameend substitution
    elem_t pattern_acts;   // complete ACTs from whenever the subject contains an "*"
    context_t *context;    // the input context
    FILE *out;             // the output file for this container
    FILE *err;             // the output file for errors for this container

#ifdef EMIT_TERM
    char unterm;           // used to emit TERM events even when no explicit ';' was in the input
			   //    (rarely used, since it has virtally zero value in outputs either)
#endif

    // FIXME  - place for fork header for layout process...
    
};
