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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#include "types.h"
#include "fatal.h"
#include "inbuf.h"
#include "list.h"
#include "io.h"
#include "grammar.h"
#include "token.h"
#include "print.h"

#define IO() ((IO_t*)TOKEN)
#define LIST() ((LIST_t*)TOKEN)
#define INBUF() ((INBUF_t*)TOKEN)

/**
 * report an error during parsing with context info.
 *
 * @param TOKEN context
 * @param message error description
 * @param si parser state
 */
void token_error(TOKEN_t * TOKEN, char *message, state_t si)
{
    unsigned char *in = IO()->in;
    unsigned char *end = IO()->in;
    char *fn = "stdin", *q="\"";


// FIXME - print errors in g syntax, e.g.
//
// error [ prog=g user=je2641 host=work file=stdin line=123 act=456 message="snafu"]
//
// Also return act# in query responses to that call can maintain sync
//
// Don't exit prog on input errors.  flush stdin, or file input ???   cycle until
// first successful sync (QRY of some kind)
//
// answer [ prog=g user=je2641 host=work file=stdin line=123 act=456  ] { any g }
//
// need THREAD context.  answer, error attributes are the thread context.  Sync query might be just "?"
//
    if (strcmp(IO()->filename, "-")) {
        fn = IO()->filename;
    } else {
        q = "";
    }
    fprintf(IO()->err, "Error: %s ", message);
    print_len_frag(IO()->err, NAMEP(si));
    fprintf(IO()->err, "\n       from %s%s%s line: %ld while processing: \"",
        q,fn,q,
        (IO()->stat_lfcount ?
            IO()->stat_lfcount :
            IO()->stat_crcount) - IO()->linecount_at_start + 1);
    while (in != end) {
        unsigned char c = *in++;
        if (c == '\n' || c == '\r') {
            break;
        }
        putc(c, IO()->err);
    }
    fprintf(IO()->err, "\"\n");
    exit(EXIT_FAILURE);
}

/**
 * consume comment fagments (all chars to EOL)
 *
 * @param TOKEN context
 */
static void token_comment_fragment(TOKEN_t * TOKEN)
{
    unsigned char *in = IO()->in;
    unsigned char *end = IO()->end;

    while (in != end) {
        unsigned char c = *in++;
        if (c == '\n' || c == '\r') {
            break;
        }
    }
    IO()->in = in;
}

/**
 * consume all comment up to next token, or EOF
 *
 * @param TOKEN context
 * @return success/fail
 */
static success_t token_comment(TOKEN_t * TOKEN)
{
    success_t rc = SUCCESS;
    token_comment_fragment(TOKEN);      // eat comment
    while (IO()->in == IO()->end) {    // end_of_buffer, or EOF, during comment
        rc = input(IO());
        if (rc == FAIL) {
            break;                      // EOF
        }
        token_comment_fragment(TOKEN);  // eat comment
    }
    return rc;
}

/**
 * consume whitespace fagments
 *
 * @ TOKEN context
 */
static void token_whitespace_fragment(TOKEN_t * TOKEN)
{
    unsigned char *in = IO()->in;
    unsigned char *end = IO()->end;

    while (in != end) {
        unsigned char c = *in;

        if (char2state[c] != WS) {
            break;
        }
        if (c == '\n') {
            IO()->stat_lfcount++;
        }
        else if (c == '\r') {
            IO()->stat_crcount++;
        }
        in++;
    }
    IO()->in = in;
}

/**
 * consume all non-comment whitespace up to next token, or EOF
 *
 * @param TOKEN context
 * @return success/fail
 */
static success_t token_non_comment(TOKEN_t * TOKEN)
{
    success_t rc = SUCCESS;
    token_whitespace_fragment(TOKEN);     // eat whitespace
    while (IO()->in == IO()->end) {     // end_of_buffer, or EOF,
                                          //   during whitespace
        rc = input(IO());
        if (rc == FAIL) {
            break;                        // EOF
        }
        token_whitespace_fragment(TOKEN); // eat all remaining leading whitespace
    }
    return rc;
}

/**
 * consume all whitespace or comments up to next token, or EOF
 *
 * @param TOKEN context
 * @return success/fail
 */
success_t token_whitespace(TOKEN_t * TOKEN)
{
    success_t rc;

    rc = SUCCESS;
    do {
        if ((rc = token_non_comment(TOKEN)) == FAIL) {
            break;
        }
        if (IO()->in == IO()->end) {
            break;
        }
        if (*(IO()->in) != '#') {
            break;
        }
        while (IO()->in != IO()->end && *(IO()->in) == '#') {
            if ((rc = token_comment(TOKEN)) == FAIL) {
                break;
            }
        }
    } while (IO()->in != IO()->end);
    if (IO()->in == IO()->end) {
        TOKEN->insi = END;
    }
    else {
        TOKEN->insi = char2state[*(IO()->in)];
    }
    return rc;
}

/**
 * if a string is suitable, convert to a shortstr
 *
 * @param TOKEN context
 * @param slen - string length of the string
 * @param string
 */
void
token_pack_string(TOKEN_t *TOKEN, int slen, elem_t *string) {
    // string must be short
    // ( AST is not special in this )
    if (slen <= sizeof(((elem_t*)0)->u.s.str)) {
        fraglist2shortstr(LIST(), slen, string);
        TOKEN->stat_instringshort++;
    } else {
        TOKEN->stat_instringlong++;
    }
}
