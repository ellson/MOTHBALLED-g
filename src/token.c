/* vim:set shiftwidth=4 ts=8 expandtab: */

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

/**
 * report an error during parsing with context info.
 *
 * @param TOKEN context
 * @param message error description
 * @param si parser state
 */
void token_error(TOKEN_t * TOKEN, char *message, state_t si)
{
    unsigned char *p, c;
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
    p = TOKEN->in;
    while ((c = *p++)) {
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
static success_t token_more_in(TOKEN_t * TOKEN)
{
    INBUF_t *INBUF = (INBUF_t *)TOKEN;
    int size;

    if (INBUF->inbuf) {        // if there is an existing active-inbuf
        if (TOKEN->in == &(INBUF->inbuf->end_of_buf)) {    // if it is full
            if ((--(INBUF->inbuf->refs)) == 0) {    // dereference active-inbuf
                free_inbuf(INBUF, INBUF->inbuf);    // free if no refs left (unlikely)
            }
            INBUF->inbuf = new_inbuf(INBUF);    // get new
            assert(INBUF->inbuf);
            INBUF->inbuf->refs = 1;    // add active-inbuf reference
            TOKEN->in = INBUF->inbuf->buf;    // point to beginning of buffer
        }
    } else {        // no inbuf, implies just starting
        INBUF->inbuf = new_inbuf(INBUF);    // get new
        assert(INBUF->inbuf);
        INBUF->inbuf->refs = 1;    // add active-inbuf reference
        TOKEN->in = INBUF->inbuf->buf;
    }
    if (TOKEN->file) {        // if there is an existing active input file
        if (TOKEN->insi == NLL && feof(TOKEN->file)) {    //    if it is at EOF
            //   Although the grammar doesn't care, I decided that it would
            //   be more user-friendly to check that we are not in a quote string
            //   whenever EOF occurs.
            if (TOKEN->in_quote) {
                token_error(TOKEN, "EOF in the middle of a quote string", NLL);
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
    size = fread(TOKEN->in, 1, &(INBUF->inbuf->end_of_buf) - TOKEN->in, TOKEN->file);
    TOKEN->in[size] = '\0';    // ensure terminated (we have an extras
    //    character in inbuf for this )
    TOKEN->insi = char2state[*TOKEN->in];

    TOKEN->stat_incharcount += size;
    return SUCCESS;
}

/**
 * consume comment fagments
 *
 * @param TOKEN context
 */
static void token_comment_fragment(TOKEN_t * TOKEN)
{
    unsigned char *in, c;

    in = TOKEN->in;
    c = *in;
    while (c != '\0' && c != '\n' && c != '\r') {
        c = *++in;
    }
    TOKEN->insi = char2state[c];
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
    success_t rc;

    rc = SUCCESS;
    token_comment_fragment(TOKEN);    // eat comment
    while (TOKEN->insi == NLL) {    // end_of_buffer, or EOF, during comment
        if ((rc = token_more_in(TOKEN) == FAIL)) {
            break;    // EOF
        }
        token_comment_fragment(TOKEN);    // eat comment
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
    unsigned char *in, c;
    state_t insi;

    if ((in = TOKEN->in)) {
        c = *in;
        insi = TOKEN->insi;
        while (insi == WS) {    // eat all leading whitespace
            if (c == '\n') {
                TOKEN->stat_lfcount++;
            }
            if (c == '\r') {
                TOKEN->stat_crcount++;
            }
            c = *++in;
            insi = char2state[c];
        }
        TOKEN->insi = insi;
        TOKEN->in = in;
    }
}

/**
 * consume all non-comment whitespace up to next token, or EOF
 *
 * @param TOKEN context
 * @return success/fail
 */
static success_t token_non_comment(TOKEN_t * TOKEN)
{
    success_t rc;

    rc = SUCCESS;
    token_whitespace_fragment(TOKEN);    // eat whitespace
    while (TOKEN->insi == NLL) {    // end_of_buffer, or EOF, during whitespace
        if ((rc = token_more_in(TOKEN) == FAIL)) {
            break;    // EOF
        }
        token_whitespace_fragment(TOKEN);    // eat all remaining leading whitespace
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
    while (1) {
        if ((rc = token_non_comment(TOKEN)) == FAIL) {
            break;
        }
        if (TOKEN->insi != OCT) {
            break;
        }
        while (TOKEN->insi == OCT) {
            if ((rc = token_comment(TOKEN)) == FAIL) {
                break;
            }
        }
    }
    return rc;
}

/**
 * load STRING fragments
 *
 * @param TOKEN context
 * @param fraglist - list of frags constituting a string
 * @return length of string
 */
static int token_string_fragment(TOKEN_t * TOKEN, elem_t * fraglist)
{
    unsigned char *frag;
    state_t insi;
    int slen, len;
    elem_t *elem;

    slen = 0;
    while (1) {
        if (TOKEN->in_quote) {
            if (TOKEN->in_quote == 2) {    // character after BSL
                TOKEN->in_quote = 1;
                frag = TOKEN->in;
                TOKEN->insi = char2state[*++(TOKEN->in)];
                elem = new_frag((LIST_t*)TOKEN, BSL, 1, frag);
                slen++;
            } else if (TOKEN->insi == DQT) {
                TOKEN->in_quote = 0;
                TOKEN->insi = char2state[*++(TOKEN->in)];
                continue;
            } else if (TOKEN->insi == BSL) {
                TOKEN->in_quote = 2;
                TOKEN->insi = char2state[*++(TOKEN->in)];
                TOKEN->has_bsl = 1;
                continue;
            } else if (TOKEN->insi == NLL) {
                break;
            } else {
                frag = TOKEN->in;
                len = 1;
                while (1) {
                    insi = char2state[*++(TOKEN->in)];
                    if (insi == DQT || insi == BSL || insi == NLL) {
                        break;
                    }
                    len++;
                }
                TOKEN->insi = insi;
                elem = new_frag((LIST_t*)TOKEN, DQT, len, frag);
                slen += len;
            }
        } else if (TOKEN->insi == ABC) {
            frag = TOKEN->in;
            len = 1;
            while ((insi = char2state[*++(TOKEN->in)]) == ABC) {
                len++;
            }
            TOKEN->insi = insi;
            elem = new_frag((LIST_t*)TOKEN, ABC, len, frag);
            slen += len;
        } else if (TOKEN->insi == AST) {
            TOKEN->has_ast = TOKEN->is_pattern = 1;
            frag = TOKEN->in;
            while ((TOKEN->insi = char2state[*++(TOKEN->in)]) == AST) {
            }    // extra '*' ignored
            elem = new_frag((LIST_t*)TOKEN, AST, 1, frag);
            slen++;
        } else if (TOKEN->insi == DQT) {
            TOKEN->in_quote = 1;
            TOKEN->quote_state = DQT;
            TOKEN->insi = char2state[*++(TOKEN->in)];
            continue;
        } else {
            break;
        }
        append_transfer(fraglist, elem);
        TOKEN->stat_infragcount++;
    }
    return slen;
}

static elem_t *
token_pack_string(TOKEN_t *TOKEN, int slen, elem_t **fraglist) {
    elem_t *new, *old, *frag;
    unsigned char *src, *dst;
    int i;

    // string must be short and not with special AST or BSL fragments
    if (slen <= sizeof(((elem_t*)0)->u.s.str)
                && !TOKEN->has_ast && !TOKEN->has_bsl) {
        TOKEN->stat_instringshort++;
#if 0
// FIXME has_ast, has_bsl  not working

        new = *fraglist;
        old = frag = new->u.l.first;
        dst = new->u.s.str;
        new->type = SHORTSTRELEM;
        while (frag) {
            for (i = frag->len, src = frag->u.f.frag; i; --i) {
                *dst++ = *src++;
            }
            frag = frag->u.f.next;
        }
        free_list((LIST_t*)TOKEN, old);
        new->len = slen;
#endif
    } else {
        TOKEN->stat_instringlong++;
    }
    (*fraglist)->state = TOKEN->quote_state;
}

/**
 * collect fragments to form a STRING token
 *
 * @param TOKEN context
 * @param fraglist
 * @return success/fail
 */
 
success_t token_string(TOKEN_t * TOKEN, elem_t **fraglist)
{
    int len, slen;

    TOKEN->has_ast = TOKEN->has_bsl = 0;
    TOKEN->quote_state = ABC;
    slen = token_string_fragment(TOKEN, *fraglist);    // leading string
    while (TOKEN->insi == NLL) {    // end_of_buffer, or EOF, during whitespace
        if ((token_more_in(TOKEN) == FAIL)) {
            break;    // EOF
        }
        if ((len = token_string_fragment(TOKEN, *fraglist)) == 0) {
            break;
        }
        slen += len;
    }
    if (slen > 0) {
        token_pack_string(TOKEN, slen, fraglist); // may replace fraglist with a shortstr elem
        return SUCCESS;
    }
    return FAIL;
}

/**
 * load VSTRING fragments
 *
 * FIXME - add support for additonal quoting formats  (HTML-like, ...)
 *
 * @param TOKEN context
 * @param fraglist
 * @return length of string
 */
static int token_vstring_fragment(TOKEN_t * TOKEN, elem_t *fraglist)
{
    unsigned char *frag;
    state_t insi;
    int slen, len;
    elem_t *elem;

    slen = 0;
    while (1) {
        if (TOKEN->in_quote) {
            if (TOKEN->in_quote == 2) {    // character after BSL
                TOKEN->in_quote = 1;
                frag = TOKEN->in;
                TOKEN->insi = char2state[*++(TOKEN->in)];
                elem = new_frag((LIST_t*)TOKEN, BSL, 1, frag);
                slen++;
            } else if (TOKEN->insi == DQT) {
                TOKEN->in_quote = 0;
                TOKEN->insi = char2state[*++(TOKEN->in)];
                continue;
            } else if (TOKEN->insi == BSL) {
                TOKEN->in_quote = 2;
                TOKEN->insi = char2state[*++(TOKEN->in)];
                TOKEN->has_bsl = 1;
                continue;
            } else if (TOKEN->insi == NLL) {
                break;
            } else {
                frag = TOKEN->in;
                len = 1;
                while (1) {
                    insi = char2state[*++(TOKEN->in)];
                    if (insi == DQT || insi == BSL || insi == NLL) {
                        break;
                    }
                    len++;
                }
                TOKEN->insi = insi;
                elem = new_frag((LIST_t*)TOKEN, DQT, len, frag);
                slen += len;
            }
        // In the unquoted portions of VSTRING we allow '/' '\' ':' '?'
        // in addition to the ABC class
        // this allows URIs as values without quoting
        } else if (TOKEN->insi == ABC ||
                   TOKEN->insi == FSL ||
                   TOKEN->insi == BSL ||
                   TOKEN->insi == CLN ||
                   TOKEN->insi == QRY) {
            frag = TOKEN->in;
            len = 1;
            while ((insi = char2state[*++(TOKEN->in)]) == ABC ||
                    insi == FSL ||
                    insi == BSL ||
                    insi == CLN ||
                    insi == QRY) {
                len++;
            }
            TOKEN->insi = insi;
            elem = new_frag((LIST_t*)TOKEN, ABC, len, frag);
            slen += len;

        // but '*' are still special  (maybe used as wild card in queries)
        } else if (TOKEN->insi == AST) {
            TOKEN->has_ast = TOKEN->is_pattern = 1;
            frag = TOKEN->in;
            while ((TOKEN->insi = char2state[*++(TOKEN->in)]) == AST) {
            }    // extra '*' ignored
            elem = new_frag((LIST_t*)TOKEN, AST, 1, frag);
            slen++;
        } else if (TOKEN->insi == DQT) {
            TOKEN->in_quote = 1;
            TOKEN->quote_state = DQT;
            TOKEN->insi = char2state[*++(TOKEN->in)];
            continue;
        } else {
            break;
        }
        append_transfer(fraglist, elem);
        TOKEN->stat_infragcount++;
    }
    return slen;
}

/**
 * collect fragments to form a VSTRING token
 *
 * @param TOKEN context
 * @param fraglist
 * @return success/fail
 */
success_t token_vstring(TOKEN_t * TOKEN, elem_t **fraglist)
{
    int len, slen;

    TOKEN->has_ast = TOKEN->has_bsl = 0;
    TOKEN->quote_state = ABC;
    slen = token_vstring_fragment(TOKEN, *fraglist);    // leading string
    while (TOKEN->insi == NLL) {    // end_of_buffer, or EOF, during whitespace
        if ((token_more_in(TOKEN) == FAIL)) {
            break;    // EOF
        }
        if ((len = token_vstring_fragment(TOKEN, *fraglist)) == 0) {
            break;
        }
        slen += len;
    }
    if (slen > 0) {
        token_pack_string(TOKEN, slen, fraglist); // may replace fraglist with a shortstr elem
        return SUCCESS;
    } 
    return FAIL;
}

/**
 * process single character tokens
 *
 * @param TOKEN context
 * @return success/fail
 */
success_t token(TOKEN_t * TOKEN)
{
    TOKEN->insi = char2state[*++(TOKEN->in)];
    return SUCCESS;
}

/**
 * test for more repetitions
 *
 * @param TOKEN context
 * @param prop properties from grammar
 * @return success = more, fail - no more
 */
success_t token_more_rep(TOKEN_t * TOKEN, unsigned char prop)
{
    state_t ei, bi;

    if (!(prop & (REP | SREP))) {
        return FAIL;
    }
    ei = TOKEN->ei;
    if (ei == RPN || ei == RAN || ei == RBR || ei == RBE) {
        return FAIL;       // no more repetitions
    }
    bi = TOKEN->bi;
    if (bi == RPN || bi == RAN || bi == RBR || bi == RBE
        || (ei != ABC && ei != AST && ei != DQT)) {
        return SUCCESS;    // more repetitions, but additional WS sep is optional
    }
    return SUCCESS;        // more repetitions
}
