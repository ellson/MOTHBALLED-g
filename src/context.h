typedef struct emit_s emit_t;
typedef struct container_context_s container_context_t;

typedef struct {		// input_context
	char *progname;		// name of program
	int *pargc;	    	// remaining filenames from command line
	char **argv;
	char *filename;		// name of file currently being processed, or "-" for stdin
	FILE *file;	    	// open file handle for file currently being processed
	long linecount_at_start;	// activity line count when this file was opened.
	inbuf_t *inbuf;		// the active input buffer
	unsigned char *in;	// next charater to be processed
	state_t insi;		// state represented by last character read
	state_t ei;	    	// ei, bi are used to determine whitespace needs around STRINGs
	state_t bi;
	state_t state;		// last state entered
	char has_ast;		// flag set if an '*' is found in a STRING
	char in_quote;		// flag set if between "..."
	char has_quote;		// flag set if STRING contains one or more DQT fragments
	int containment;	// depth of containment
	FILE *out;	    	// the output file 
	struct timespec uptime; // seconds since boot, also used as the starttime fpr runtime calculations

} context_t;

struct container_context_s {	// container_context
	context_t *context;	// the input context
	elem_t subject; 	// Preceeding ACT's subject, until this ACT's SUBJECT has been parsed
                        // and processd by sameas()  - at which point it becomes this ACT's subject.
                        // (So: in SUBJECT parsing it is the previous ACT's subject and used for sameas()
                        // substitutions once a new SUBJECT has been parsed. For ATTRIBUTES
                        // and CONTAINERS it is this ACT.   It is the basis of the name for
                        // the output files for contents.)
	char is_pattern;	// flag set if '*' occurred in SUBJECT
	state_t act_type;	// set by sameas() to record if the SUBJECT is NODE(s), or EDGE(s),
                        //   and to check that it is not a mix of NODE(s) and EDGE(s).
	elem_t node_pattern_acts;	// complete ACTs from whenever the NODE subject contains an "*"
	elem_t edge_pattern_acts;	// complete ACTs from whenever the EDGE subject contains an "*"
	char sep;       	// the next separator (either 0, or ' ' if following a STRING that requires a separator.
                        //   may be ignored if the next character is a token which implicitly separates.)
    char style;         // normal or SHELL_FRIENDLY  // FIXME use enum with additional styles
    char hashname[12];  // base-64 ascii of 64bit hash of subject, '\0' terminated, for use as filename for contents
	FILE *out;	    	// the output file for this container

	// FIXME  - place for fork header for layout process...

};

// FIXME - use an enum for styles
//       - other styles:    newline per ACT
//                          newline per sameas() ACT set  (new ACT has no EQL in SUBJECT)
#define SHELL_FRIENDLY_STYLE 1
