/* vim:set shiftwidth=4 ts=8 expandtab: */

typedef struct input_s input_t;

struct input_s {
    int *pargc;                // remaining filenames from command line
    char **argv;
    char *filename;            // name of file currently being processed, or "-" for stdin
    FILE *file;                // open file handle for file currently being processed
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
};

void je_parse_error(input_t * IN, state_t si, char *message);
success_t je_parse_whitespace(context_t * C);
success_t je_parse_string(context_t * C, elem_t * fraglist);
success_t je_parse_vstring(context_t * C, elem_t * fraglist);
success_t je_parse_token(context_t * C);
