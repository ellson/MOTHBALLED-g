typedef struct container_context_s container_context_t;

typedef struct {           // input_context
    int *pargc;            // remaining filenames from command line
    char **argv;
    char *filename;        // name of file currently being processed, or "-" for stdin
    FILE *file;            // open file handle for file currently being processed
    inbuf_t *inbuf;        // the active input buffer
    unsigned char *in;     // next charater to be processed
    state_t insi;          // state represented by last character read
    int containment;       // depth of containment
    FILE *out;             // the output file 
    FILE *err;             // the output file for errors
    container_context_t *container_context;
} context_t;

struct container_context_s { // container_context (also output context)
    elem_t prev_subject;   // preceeding ACT's subject,  for sameend substitution
    elem_t pattern_acts;   // complete ACTs from whenever the subject contains an "*"
    context_t *context;    // the input context
    FILE *out;             // the output file for this container
    FILE *err;             // the output file for errors for this container
    state_t subj;          // used to verify homogenous SUBJECT


    // FIXME  - place for fork header for layout process...
    
};
