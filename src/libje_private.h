#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

#include "grammar.h"
#include "libje.h"

typedef struct inbuf_s inbuf_t;
typedef struct emit_s emit_t;
typedef struct container_context_s container_context_t;
typedef struct hashfile_s hashfile_t;

// sizeof(inbuf_t) = 1<<7  (128 bytes)
// the size of .buf is sizeof(inbuf_t) less the other bits  (~115 bytes, I think)
#define INBUFSIZE ((1<<7) - sizeof(inbuf_t*) - sizeof(int) - sizeof(char))
#define INBUFALLOCNUM 128

struct inbuf_s {
	inbuf_t *next;
	int refs;
	unsigned char buf[INBUFSIZE];
	unsigned char end_of_buf;	// maintain a '\0' here 
};

// LISTELEM must = 0 for static or calloc allocation of list headers
typedef enum {
	LISTELEM = 0,
	FRAGELEM = 1,
	HASHELEM = 2
} elemtype_t;

struct elem_s {
	elem_t *next;
	union {
		struct {
			elem_t *first;	// for push, pop, and forward walk
			elem_t *last;	// for append
		} list;
		struct {
			inbuf_t *inbuf;	// inbuf containing frag
			unsigned char *frag;	// point to beginning of frag
		} frag;
		struct {
			unsigned long hash; // hash value
			FILE *out;      // file handle, or NULL if not opened yet.
		} hash;
	} u;
	// FIXME -- There must be a better way ?
	//          If these "ref" or "len" ints are included in the above union{}, then
	//          the size of elem_t increases from 32 to 40 bytes.
	union {
		struct {
			int refs;	// reference count
		} list;
		struct {
			int len;	// length of frag
		} frag;
	} v;
	// FIXME -- can't store type as elemtype_t, or state as state_t, or it would assume ints.
	char type;		// LISTELEM or FRAGELEM
	char state;		// state_machine state that generated this list
};

#define size_elem_t (sizeof(elem_t*)*((sizeof(elem_t)+sizeof(elem_t*)-1)/(sizeof(elem_t*))))
#define LISTALLOCNUM 512

struct hashfile_s {
    hashfile_t *next;
    FILE *out;
    long hash;
};

// emit styles
typedef enum {
	MINIMUM_SPACE_STYLE = 0,
	SHELL_FRIENDLY_STYLE = 1
} style_t;

struct context_s {		// input_context
	char *progname;		// name of program
	int *pargc;	    	// remaining filenames from command line
	char **argv;
	char *filename;		// name of file currently being processed, or "-" for stdin
	FILE *file;	    	// open file handle for file currently being processed
	FILE *out;	    	// typically stdout for parser debug outputs
	inbuf_t *inbuf;		// the active input buffer
	unsigned char *in;	// next charater to be processed
    char *username;     // set by first call to g_session
    char *hostname;     // ditto
    char *tempdir;      // temporary dir for container files
    inbuf_t *free_inbuf_list; // linked list of unused inbufs
    elem_t *free_elem_list; // linked list of unused list elems
	state_t insi;		// state represented by last character read
	state_t ei;	    	// ei, bi are used to determine whitespace needs around STRINGs
	state_t bi;
	state_t verb;       // the "verb" for the ACT. Default is "add",
                                                    // '~' is "delete",
                                                    // '?' is "query"
	state_t state;		// last state entered
	char has_ast;		// flag set if an '*' is found in a STRING
	char has_cousin;	// flag set if a COUSIN is found in any EDGE of the ACT (forward EDGE to ancestors for processing)
	char in_quote;		// flag set if between "..."
	char has_quote;		// flag set if STRING contains one or more DQT fragments
	char needstats;		// flag set if -s on command line
	char sep;       	// the next separator (either 0, or ' ' if following a STRING that requires a separator.
                        //   may be ignored if the next character is a token which implicitly separates.)
    style_t style;      // spacing style in emitted outputs
                        //   may be ignored if the next character is a token which implicitly separates.)
	int containment;	// depth of containment
    char template[32];  // place to keep template for mkdtemp()
	long linecount_at_start;    // activity line count when this file was opened.
    elem_t myname;      // header for a list of components of my name (in the same form as subjects)
    elem_t *hash_buckets[64];   // 64 buckets of name hashes and FILE*.
    long stat_filecount;  // various stats
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
	struct timespec uptime; // seconds since boot, also used as the starttime fpr runtime calculations
    pid_t pid;
};

struct container_context_s {	// container_context
	context_t *context;	// the input context
	elem_t subject; 	// Preceeding ACT's subject, until this ACT's SUBJECT has been parsed
                        // and processd by sameas()  - at which point it becomes this ACT's subject.
                        // (So: in SUBJECT parsing it is the previous ACT's subject and used for sameas()
                        // substitutions once a new SUBJECT has been parsed. For ATTRIBUTES
                        // and CONTAINERS it is this ACT.   It is the basis of the name for
                        // the output files for contents.)
	char is_pattern;	// flag set if '*' occurred in SUBJECT
	state_t subject_type;	// set by sameas() to record if the SUBJECT is NODE(s), or EDGE(s),
                        //   and to check that it is not a mix of NODE(s) and EDGE(s).
	elem_t node_pattern_acts;	// complete ACTs from whenever the NODE subject contains an "*"
	elem_t edge_pattern_acts;	// complete ACTs from whenever the EDGE subject contains an "*"
	FILE *out;	    	// the output file for this container

	// FIXME  - place for fork header for layout process...

};

struct emit_s {
    char *name;
	void (*initialize) (context_t * C);
	void (*finalize) (context_t * C);

	void (*start_file) (context_t * C);
	void (*end_file) (context_t * C);

	void (*start_activity) (container_context_t * CC);
	void (*end_activity) (container_context_t * CC);

	void (*start_act) (container_context_t * CC);
	void (*end_act) (container_context_t * CC);

	void (*start_subject) (container_context_t * CC);
	void (*end_subject) (container_context_t * CC);

	void (*start_state) (container_context_t * CC, char class, unsigned char prop, int nest, int repc);
	void (*end_state) (container_context_t * CC, char class, success_t rc, int nest, int repc);

	void (*act) (container_context_t * CC, elem_t * root);
	void (*subject) (container_context_t * CC, elem_t * root);
	void (*attributes) (container_context_t * CC, elem_t * root);

	void (*sep) (context_t * C);
	void (*token) (context_t * C, char token);
	void (*string) (context_t * C, elem_t * branch);
	void (*frag) (context_t * C, unsigned char len, unsigned char *frag);
	void (*error) (context_t * C, state_t si, char *message);
};

#define emit_initialize(C) \
    if (emit->initialize) {emit->initialize(C);}
#define emit_finalize(C) \
    if (emit->finalize) {emit->finalize(C);}

#define emit_start_file(C) \
    if (emit->start_file) {emit->start_file(C);}
#define emit_end_file(C) \
    if (emit->end_file) {emit->end_file(C);}

#define emit_start_activity(CC) \
    if (emit->start_activity) {emit->start_activity(CC);}
#define emit_end_activity(CC) \
    if (emit->end_activity) {emit->end_activity(CC);}

#define emit_start_act(CC) \
    if (emit->start_act) {emit->start_act(CC);}
#define emit_end_act(CC) \
    if (emit->end_act) {emit->end_act(CC);}

#define emit_start_subject(CC) \
    if (emit->start_subject) {emit->start_subject(CC);}
#define emit_end_subject(CC) \
    if (emit->end_subject) {emit->end_subject(CC);}

#define emit_start_state(C, class, prop, nest, repc) \
    if (emit->start_state) {emit->start_state(C, class, prop, nest, repc);}
#define emit_end_state(C, class, rc, nest, repc) \
    if (emit->end_state) {emit->end_state(C, class, rc, nest, repc);}
#define emit_act(CC, root) \
    if (emit->act) {emit->act(CC, root);}
#define emit_subject(CC, root) \
    if (emit->subject) {emit->subject(CC, root);}
#define emit_attributes(CC, root) \
    if (emit->attributes) {emit->attributes(CC, root);}

#define emit_sep(C) \
    if (emit->sep) {emit->sep(C);}
#define emit_token(C, token) \
    if (emit->token) {emit->token(C, token);}
#define emit_string(C, branch) \
    if (emit->string) {emit->string(C, branch);}
#define emit_frag(C, len, frag) \
    if (emit->frag) {emit->frag(C, len, frag);}
#define emit_error(C, si, message) \
    if (emit->error) {emit->error(C, si, message);}

// if we're not providing the function in any api,
//    then we can avoid the runtime cost of testing for it
#undef emit_start_act
#define emit_start_act(CC, len, frag)

#undef emit_end_act
#define emit_end_act(CC, len, frag)

#undef emit_start_subject
#define emit_start_subject(CC, len, frag)

#undef emit_end_subject
#define emit_end_subject(CC, len, frag)

#undef emit_frag
#define emit_frag(C, len, frag)

// emit.c
extern emit_t *emit;
extern emit_t g_api, g1_api, g2_api, t_api, t1_api, gv_api;
char je_char_prop(unsigned char prop, char noprop);
void je_append_token(context_t *C, char **pos, char tok);
void je_append_string(context_t *C, char **pos, char *string);
void je_append_ulong(context_t *C, char **pos, unsigned long integer);
void je_append_runtime(context_t *C, char **pos, unsigned long run_sec, unsigned long run_ns);
void je_emit_list(context_t * C, FILE * chan, elem_t * subject);
void je_emit_error(context_t * CC, state_t si, char *message);

// inbuf.c
void new_inbuf(context_t * C);
void free_inbuf(context_t * C, inbuf_t * inbuf);

// list.c
elem_t *new_hash(context_t * C, unsigned long hash);
elem_t *new_frag(context_t * C, char state, int len, unsigned char *frag);
elem_t *move_list(context_t * C, elem_t * list);
elem_t *ref_list(context_t * C, elem_t * list);
void append_list(elem_t * list, elem_t * elem);
void free_list(context_t * C, elem_t * list);
int print_len_frag(FILE * chan, unsigned char *len_frag);
void print_frags(FILE * chan, state_t liststate, elem_t * elem, char *sep);
void print_list(FILE * chan, elem_t * list, int nest, char *sep);

// hash.c
elem_t * je_hash_bucket(context_t * C, unsigned long hash);
void je_hash_list(unsigned long *hash, elem_t *list);
char * je_long_to_base64(unsigned long *phash);
success_t je_base64_to_long(char *b64string, unsigned long *phash);

// token.c
success_t je_parse_whitespace(context_t * C);
success_t je_parse_string(context_t * C, elem_t * fraglist);
success_t je_parse_vstring(context_t * C, elem_t * fraglist);
success_t je_parse_token(context_t * C);

// pattern.c
void je_pattern(container_context_t * CC, elem_t * root, elem_t * subject);

// sameas.c
void je_sameas(container_context_t * CC, elem_t * subject);

// dispatch.c
void je_dispatch(container_context_t * CC, elem_t * root);
