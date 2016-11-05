/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef TOKEN_H
#define TOKEN_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    LIST_t LIST;               // LIST context. Maybe cast from TOKEN 
    int *pargc;                // remaining filenames from command line
    char **argv;
    char *filename;            // name of file currently being processed, or "-" for stdin
    FILE *file;                // open file handle for file currently being processed
    FILE *out, *err;           // output files
    unsigned char *in;         // next character to be processed
    state_t insi;              // state represented by last character read
    state_t ei;                // ei, bi are used to determine whitespace needs around STRINGs
    state_t bi;
    state_t state;             // last state entered
    state_t quote_state;       // ABC or DQT, DQT if STRING contains DQT fragments
    char in_quote;             // flag set if between "..."
    char has_ast;              // flag set if an '*' is found in a STRING
    long linecount_at_start;   // activity line count when this file was opened.
    long stat_lfcount;
    long stat_crcount;
    long stat_incharcount;
    long stat_infragcount;
    long stat_instringshort;
    long stat_instringlong;
    long stat_infilecount;       // various stats
} TOKEN_t;

void token_error(TOKEN_t * TOKEN, char *message, state_t si);
success_t token_whitespace(TOKEN_t * TOKEN);
success_t token_string(TOKEN_t * TOKEN, elem_t * fraglist);
success_t token_vstring(TOKEN_t * TOKEN, elem_t * fraglist);
success_t token(TOKEN_t * TOKEN);

#ifdef __cplusplus
}
#endif

#endif
