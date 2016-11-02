/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef TOKEN_H
#define TOKEN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "libje.h"
#include "frag.h"

typedef struct {
    LIST_t LIST;             // Must be first (to allow casting from PARSE_t)

    int *pargc;                // remaining filenames from command line
    char **argv;
    char *filename;            // name of file currently being processed, or "-" for stdin
    FILE *file;                // open file handle for file currently being processed
    unsigned char *in;         // next charater to be processed
    state_t insi;              // state represented by last character read
    state_t ei;                // ei, bi are used to determine whitespace needs around STRINGs
    state_t bi;
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
} TOKEN_t;

void token_error(TOKEN_t * TOKEN, state_t si, char *message);
success_t token_whitespace(TOKEN_t * TOKEN);
success_t token_string(TOKEN_t * TOKEN, elem_t * fraglist);
success_t token_vstring(TOKEN_t * TOKEN, elem_t * fraglist);
success_t token(TOKEN_t * TOKEN);

#ifdef __cplusplus
}
#endif

#endif
