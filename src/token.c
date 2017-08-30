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
#include "grammar.h"
#include "token.h"
#include "print.h"

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
    unsigned char *in = TOKEN->in;
    unsigned char *end = TOKEN->in;
    unsigned char c;
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
    if (strcmp(TOKEN->filename, "-")) {
        fn = TOKEN->filename;
    } else {
        q = "";
    }
    fprintf(TOKEN->err, "Error: %s ", message);
    print_len_frag(TOKEN->err, NAMEP(si));
    fprintf(TOKEN->err, "\n       from %s%s%s line: %ld while processing: \"",
        q,fn,q,
        (TOKEN->stat_lfcount ?
            TOKEN->stat_lfcount :
            TOKEN->stat_crcount) - TOKEN->linecount_at_start + 1);
    while (in != end) {
        c = *in++;
        if (c == '\n' || c == '\r') {
            break;
        }
        putc(c, TOKEN->err);
    }
    fprintf(TOKEN->err, "\"\n");
    exit(EXIT_FAILURE);
}

/**
 * fill buffers from input files
 *
 * @param TOKEN context
 * @return success/fail
 */
success_t token_more_in(TOKEN_t * TOKEN)
{
    int size, avail;

    if (INBUF()->inbuf) {        // if there is an existing active-inbuf
        if (TOKEN->in == (INBUF()->inbuf->buf + INBUFSIZE)) {    // if it is full
            if ((--(INBUF()->inbuf->refs)) == 0) {    // dereference active-inbuf
                free_inbuf(INBUF(), INBUF()->inbuf);    // free if no refs left
                         // can happen it held only framents that were ignore, or
                         // which were copied out into SHORTSTRELEM
            }
            INBUF()->inbuf = new_inbuf(INBUF());    // get new
            assert(INBUF()->inbuf);
            INBUF()->inbuf->refs = 1;    // add active-inbuf reference
            TOKEN->in = INBUF()->inbuf->buf;    // point to beginning of buffer
        }
    } else {        // no inbuf, implies just starting
        INBUF()->inbuf = new_inbuf(INBUF());    // get new
        assert(INBUF()->inbuf);
        INBUF()->inbuf->refs = 1;    // add active-inbuf reference
        TOKEN->in = INBUF()->inbuf->buf;
    }
    if (TOKEN->file) {        // if there is an existing active input file
        if (TOKEN->in == TOKEN->end && feof(TOKEN->file)) {    //    if it is at EOF
            //   Although the grammar doesn't care, I decided that it would
            //   be more user-friendly to check that we are not in a quote string
            //   whenever EOF occurs.
            if (TOKEN->quote_state) {
                token_error(TOKEN, "EOF in the middle of a quote string", END);
            }
// FIXME don't close stdin
// FIXME - stall for more more input   (inotify events ?)
            fclose(TOKEN->file);    // then close it and indicate no active input file
            TOKEN->file = NULL;
        }
    }
    if (!TOKEN->file) {        // if there is no active input file
        if (*(TOKEN->pargc) > 0) {    //   then try to open the next file
            TOKEN->filename = TOKEN->argv[0];
            (*(TOKEN->pargc))--;
            TOKEN->argv = &(TOKEN->argv[1]);
            if (strcmp(TOKEN->filename, "-") == 0) {
                TOKEN->file = stdin;
                *(TOKEN->pargc) = 0;    // No files after stdin
            } else if (strcmp(TOKEN->filename, "-e") == 0) {
                TOKEN->membuf = TOKEN->argv[0];
                (*(TOKEN->pargc))--;
                if (TOKEN->membuf) fprintf(stderr, "NOT YET WORKING:  %s\n", TOKEN->membuf);
                    return FAIL;    // no more input available
            } else {
                if (! (TOKEN->file = fopen(TOKEN->filename, "rb")))
                    FATAL("fopen(\"%s\", \"rb\")", TOKEN->filename);
            }
            TOKEN->linecount_at_start = TOKEN->stat_lfcount ? TOKEN->stat_lfcount : TOKEN->stat_crcount;
            TOKEN->stat_infilecount++;
        } else {
            return FAIL;    // no more input available
        }
        assert(TOKEN->file);
    }
    // slurp in data from file stream
    avail = INBUF()->inbuf->buf + INBUFSIZE - TOKEN->in;
    size = fread(TOKEN->in, 1, avail, TOKEN->file);
    TOKEN->end = TOKEN->in + size;

    if (size == 0 && ! feof(TOKEN->file)) {
        if (ferror(TOKEN->file)) {
            FATAL("fread()");
        }
    }

    TOKEN->stat_incharcount += size;
    return SUCCESS;
}

/**
 * consume comment fagments (all chars to EOL)
 *
 * @param TOKEN context
 */
static void token_comment_fragment(TOKEN_t * TOKEN)
{
    unsigned char *in = TOKEN->in;
    unsigned char *end = TOKEN->end;
    unsigned char c;

    while (in != end) {
        c = *in++;
        if (c == '\n' || c == '\r') {
            break;
        }
    }
    TOKEN->in = in;
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
    while (TOKEN->in == TOKEN->end) {    // end_of_buffer, or EOF, during comment
        rc = token_more_in(TOKEN);
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
    unsigned char *in = TOKEN->in;
    unsigned char *end = TOKEN->end;

    while (in != end) {
        unsigned char c = *in;

        if (char2state[c] != WS) {
            break;
        }
        if (c == '\n') {
            TOKEN->stat_lfcount++;
        }
        else if (c == '\r') {
            TOKEN->stat_crcount++;
        }
        in++;
    }
    TOKEN->in = in;
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
    while (TOKEN->in == TOKEN->end) {     // end_of_buffer, or EOF,
                                          //   during whitespace
        rc = token_more_in(TOKEN);
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
    while (TOKEN->in != TOKEN->end) {
        if ((rc = token_non_comment(TOKEN)) == FAIL) {
            break;
        }
        if (TOKEN->in == TOKEN->end) {
            break;
        }
        if (*(TOKEN->in) != '#') {
            break;
        }
        while (TOKEN->in != TOKEN->end && *(TOKEN->in) == '#') {
            if ((rc = token_comment(TOKEN)) == FAIL) {
                break;
            }
        }
    }
    if (TOKEN->in == TOKEN->end) {
        TOKEN->insi = END;
    }
    else {
        TOKEN->insi = char2state[*(TOKEN->in)];
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
