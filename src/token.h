/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef TOKEN_H
#define TOKEN_H

#ifdef __cplusplus
extern "C" {
#endif

struct token_s {
    LIST_t LIST;               // LIST context. Maybe cast from TOKEN 
    int *pargc;                // remaining filenames from command line
    char **argv;
    char *filename;            // name of file currently being processed,
                               //   or "-" for stdin
    FILE *file;                // file handle of file being processed
    FILE *out, *err;           // output files
    unsigned char *in;         // next character to be processed
    state_t insi;              // state represented by last character read
    state_t state;             // last state entered
    state_t quote_state;       // ABC or
                               // DQT if STRING contains any DQT fragments
    state_t pattern;           // flag set if an '*' is found in any STRING
                               //   -- reset by parse()
    state_t has_ast;           // flag set if an '*' is found in a STRING
    state_t has_bsl;           // flag set if an '\' is found in a STRING
    int in_quote;              // 0 not in quotes
                               // 1 between DQT
                               // 2 char following BSL between DQT

    long linecount_at_start;   // line count when this file was opened.
                               //   -- used to calulate line # within file

    long stat_lfcount;         // various stats
    long stat_crcount;
    long stat_incharcount;
    long stat_infragcount;
    long stat_instringshort;
    long stat_instringlong;
    long stat_infilecount; 
};

void token_error(TOKEN_t * TOKEN, char *message, state_t si);
success_t token_whitespace(TOKEN_t * TOKEN);
success_t token_string(TOKEN_t * TOKEN, elem_t *string);
success_t token_vstring(TOKEN_t * TOKEN, elem_t *string);
state_t token(TOKEN_t * TOKEN);

#ifdef __cplusplus
}
#endif

#endif
