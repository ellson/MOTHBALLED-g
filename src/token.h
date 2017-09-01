/* vim:set shiftwidth=4 ts=8 expandtab: */

/*************************************************************************
 * Copyright (c) 2017 AT&T Intellectual Property
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: John Ellson <john.ellson@gmail.com>
 *************************************************************************/

#ifndef TOKEN_H
#define TOKEN_H

#ifdef __cplusplus
extern "C" {
#endif

// extra state_t for end-of-file condition
#define gEOF 0xff
// extra state_t for end-of-stream (stalled input) condition
#define gEOS 0xfe

struct token_s {
    LIST_t LIST;               // LIST context. Maybe cast from TOKEN

    // FIXME - Why do we have these ?  Why not leave in PROCESS ?
    int *pargc;                // remaining filenames from command line
    char **argv;
    FILE *out, *err;           // output files
    char *acts;                // g snippet from command line

    unsigned char *in;         // next character to be processed
    unsigned char *end;        // one past the last character
    char *filename;            // name of file currently being processed,
                               //   or "-" for stdin
    FILE *file;                // file handle of file being processed
    char *membuf;              // An externally memory-managed, in-memory,
                               // NUL-terminated string providing ACT(s),
                               // e.g. following -e on the commandline
   

    state_t insi;              // state represented by last character read
    state_t state;             // last state entered
    state_t elem_has_ast;      // flag set if an '*' is found in any elem
                               //   -- reset by parse(), so parse defines "elem"
    state_t quote_type;        // DQT, or LPN, LBE, LAN, LBR
    int quote_state;           // 0 not in quotes
                               // 1 between DQT
                               // 2 char following BSL between DQT
    int quote_nest;            // paren nesting level, or tranparent char count
    int quote_length;          // length of transparent string

    long linecount_at_start;   // line count when this file was opened.
                               //   -- used to calculate line # within file

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
success_t token_more_in(TOKEN_t *TOKEN);
void token_pack_string(TOKEN_t *TOKEN, int slen, elem_t *string);


#ifdef __cplusplus
}
#endif

#endif
