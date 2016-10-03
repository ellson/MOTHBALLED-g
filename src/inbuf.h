/* vim:set shiftwidth=4 ts=8 expandtab: */

typedef struct inbuf_s inbuf_t;
typedef struct input_s input_t;

// sizeof(inbuf_t) = 1<<7  (128 bytes)
// the size of .buf is sizeof(inbuf_t) less the other bits  (~115 bytes, I think)
#define INBUFSIZE ((1<<7) - sizeof(inbuf_t*) - sizeof(int) - sizeof(char))
#define INBUFALLOCNUM 128

struct inbuf_s {
    inbuf_t *next;
    int refs;
    unsigned char buf[INBUFSIZE];
    unsigned char end_of_buf;    // maintain a '\0' here 
};

struct input_s {
    int *pargc;                // remaining filenames from command line
    char **argv;
    char *filename;            // name of file currently being processed, or "-" for stdin
    FILE *file;                // open file handle for file currently being processed
    inbuf_t *inbuf;            // the active input buffer
    inbuf_t *free_inbuf_list;  // linked list of unused inbufs
    unsigned char *in;         // next charater to be processed
    state_t insi;              // state represented by last character read
    state_t ei;                // ei, bi are used to determine whitespace needs around STRINGs
    state_t bi;
    state_t verb;              // the "verb" for the ACT. Default is "add",
                                                    // '~' is "delete",
                                                    // '?' is "query"
    state_t state;             // last state entered
    char in_quote;             // flag set if between "..."
    char has_ast;              // flag set if an '*' is found in a STRING
    char has_quote;            // flag set if STRING contains one or more DQT fragments
    long linecount_at_start;   // activity line count when this file was opened.
    long stat_inchars;
    long stat_lfcount;
    long stat_crcount;
    long stat_stringcount;
    long stat_fragcount;
    long stat_filecount;       // various stats
    long stat_inbufmalloc;
    long stat_inbufmax;
    long stat_inbufnow;
};

inbuf_t * new_inbuf(input_t * IN);
void free_inbuf(input_t * IN, inbuf_t * inbuf);
