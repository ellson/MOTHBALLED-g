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

#define LIST() ((LIST_t*)TOKEN)

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

// FIXME
if (TOKEN->acts) fprintf(stderr, "NOT YET WORKING:  %s\n", TOKEN->acts);

    if (INBUF->inbuf) {        // if there is an existing active-inbuf
        if (TOKEN->in == &(INBUF->inbuf->end_of_buf)) {    // if it is full
            if ((--(INBUF->inbuf->refs)) == 0) {    // dereference active-inbuf
                free_inbuf(INBUF, INBUF->inbuf);    // free if no refs left
                         // can happen it held only framents that were ignore, or
                         // which were copied out into SHORTSTRELEM
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
    success_t rc = SUCCESS;
    token_comment_fragment(TOKEN);      // eat comment
    while (TOKEN->insi == NLL) {        // end_of_buffer, or EOF, during comment
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
    unsigned char *in;
    state_t insi;

    if ((in = TOKEN->in)) {
        unsigned char c = *in;
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
    success_t rc = SUCCESS;
    token_whitespace_fragment(TOKEN);     // eat whitespace
    while (TOKEN->insi == NLL) {          // end_of_buffer, or EOF,
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

// I wanted to turn this off, to make it more C likes, and to enforce a 
// cleaner coding stle, IMHO.   Pragmatically though, if we want to allow
// translations from other languages, like DOT or JSON, thane we
// will need to support quoting in identifiers.

#define QUOTING_IN_IDENTIFIERS 1

/**
 * load IDENTIFIER fragments
 *
 * @param TOKEN context
 * @param identifier - list of frags constituting a identifier
 * @return length of identifier
 */
static int token_identifier_fragment(TOKEN_t * TOKEN, elem_t * identifier)
{
    unsigned char *frag;
    state_t insi;
    int slen, len;
    elem_t *elem;

    slen = 0;
    while (1) {
#ifdef QUOTING_IN_IDENTIFIERS
        if (TOKEN->in_quote) {
            if (TOKEN->in_quote == 2) {    // character after BSL
                TOKEN->in_quote = 1;
                frag = TOKEN->in;
                TOKEN->insi = char2state[*++(TOKEN->in)];
                elem = new_frag(LIST(), BSL, 1, frag);
                slen++;
            } else if (TOKEN->insi == DQT) {
                TOKEN->in_quote = 0;
                TOKEN->insi = char2state[*++(TOKEN->in)];
                continue;
            } else if (TOKEN->insi == BSL) {
                TOKEN->in_quote = 2;
                TOKEN->insi = char2state[*++(TOKEN->in)];
                TOKEN->has_bsl = BSL;
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
                elem = new_frag(LIST(), DQT, len, frag);
                slen += len;
            }
        } else if (TOKEN->insi == DQT) {
            TOKEN->in_quote = 1;
            TOKEN->quote_state = DQT;
            TOKEN->insi = char2state[*++(TOKEN->in)];
            continue;
        } else
#endif
        if (TOKEN->insi == ABC) {
            frag = TOKEN->in;
            len = 1;
            while ((insi = char2state[*++(TOKEN->in)]) == ABC) {
                len++;
            }
            TOKEN->insi = insi;
            elem = new_frag(LIST(), ABC, len, frag);
            slen += len;
        } else if (TOKEN->insi == AST) {
            TOKEN->has_ast = AST;
            TOKEN->elem_has_ast = AST;
            frag = TOKEN->in;
            while ((TOKEN->insi = char2state[*++(TOKEN->in)]) == AST) {
            }    // extra '*' ignored
            elem = new_frag(LIST(), AST, 1, frag);
            slen++;
        } else {
            break;
        }
        append_transfer(identifier, elem);
        TOKEN->stat_infragcount++;
    }
    return slen;
}

/**
 * if a string is suitable, convert to a shortstr
 *
 * @param TOKEN context
 * @param slen - string length of the string
 * @param string
 */
static void
token_pack_string(TOKEN_t *TOKEN, int slen, elem_t *string) {
    // string must be short and not with special BSL fragments
    // ( AST is not special this )
    if (slen <= sizeof(((elem_t*)0)->u.s.str) && !TOKEN->has_bsl) {
        fraglist2shortstr(LIST(), slen, string);
        TOKEN->stat_instringshort++;
    } else {
        TOKEN->stat_instringlong++;
    }
    string->state = TOKEN->quote_state;
}

/**
 * collect fragments to form an IDENTIFIER token
 *
 * @param TOKEN context
 * @param identifier
 * @return success/fail
 */

success_t token_identifier(TOKEN_t * TOKEN, elem_t *identifier)
{
    assert(identifier);
    assert(identifier->type == (char)LISTELEM);
    assert(identifier->refs > 0);

    TOKEN->has_ast = 0;
    TOKEN->has_bsl = 0;
    TOKEN->quote_state = ABC;
    int slen = token_identifier_fragment(TOKEN, identifier); // leading fragment
    while (TOKEN->insi == NLL) {    // end_of_buffer, or EOF, during whitespace
        if ((token_more_in(TOKEN) == FAIL)) {
            break;    // EOF
        }
        int len = token_identifier_fragment(TOKEN, identifier);
        if (len == 0) {
            break;
        }
        slen += len;
    }
    if (slen > 0) {
        token_pack_string(TOKEN, slen, identifier); // may replace identifier with a shortstr elem
        return SUCCESS;
    }
    return FAIL;
}

/**
 * load VSTRING fragments
 *
 * FIXME - add support for additonal quoting formats:
 *     HTML-like.  < and > must be properly nested
 *            <....>
 *     Binary. "length" bytes are completely transparent after the ']'
 *            [length]...
 *
 * @param TOKEN context
 * @param string
 * @return length of string
 */
static int token_vstring_fragment(TOKEN_t * TOKEN, elem_t *string)
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
                TOKEN->insi = char2vstate[*++(TOKEN->in)];
                elem = new_frag(LIST(), BSL, 1, frag);
                slen++;
            } else if (TOKEN->insi == DQT) {
                TOKEN->in_quote = 0;
                TOKEN->insi = char2vstate[*++(TOKEN->in)];
                continue;
            } else if (TOKEN->insi == BSL) {
                TOKEN->in_quote = 2;
                TOKEN->insi = char2vstate[*++(TOKEN->in)];
                TOKEN->has_bsl = BSL;
                continue;
            } else if (TOKEN->insi == NLL) {
                break;
            } else {
                frag = TOKEN->in;
                len = 1;
                while (1) {
                    insi = char2vstate[*++(TOKEN->in)];
                    if (insi == DQT || insi == BSL || insi == NLL) {
                        break;
                    }
                    len++;
                }
                TOKEN->insi = insi;
                elem = new_frag(LIST(), DQT, len, frag);
                slen += len;
            }
        } else if (TOKEN->insi == DQT) {
            TOKEN->in_quote = 1;
            TOKEN->quote_state = DQT;
            TOKEN->insi = char2vstate[*++(TOKEN->in)];
            continue;
        } else if (TOKEN->insi == ABC) {
            frag = TOKEN->in;
            len = 1;
            while ((insi = char2vstate[*++(TOKEN->in)]) == ABC) {
                len++;
            }
            TOKEN->insi = insi;
            elem = new_frag(LIST(), ABC, len, frag);
            slen += len;
        // but '*' are still special  (maybe used as wild card in queries)
        } else if (TOKEN->insi == AST) {
            TOKEN->has_ast = AST;
            TOKEN->elem_has_ast = AST;
            frag = TOKEN->in;
            while ((TOKEN->insi = char2vstate[*++(TOKEN->in)]) == AST) {
            }    // extra '*' ignored
            elem = new_frag(LIST(), AST, 1, frag);
            slen++;
        } else {
            break;
        }
        append_transfer(string, elem);
        TOKEN->stat_infragcount++;
    }
    return slen;
}

/**
 * collect fragments to form a VSTRING token
 *
 * @param TOKEN context
 * @param string
 * @return success/fail
 */
success_t token_vstring(TOKEN_t * TOKEN, elem_t *string)
{
    assert(string);
    assert(string->type == (char)LISTELEM);
    assert(string->refs > 0);
    TOKEN->has_ast = 0;
    TOKEN->has_bsl = 0;

    TOKEN->quote_state = ABC;
    TOKEN->insi = char2vstate[*(TOKEN->in)]; // recheck the first char against expanded set
    if ( ! (TOKEN->insi == ABC || TOKEN->insi == DQT || TOKEN->insi == AST || TOKEN->insi == BSL)) {
         token_error(TOKEN, "Malformed VSTRING", TOKEN->insi);
    }
    int slen = token_vstring_fragment(TOKEN, string);    // leading string
    while (TOKEN->insi == NLL) {    // end_of_buffer, or EOF, during whitespace
        if ((token_more_in(TOKEN) == FAIL)) {
            break;    // EOF
        }
        int len = token_vstring_fragment(TOKEN, string);
        if (len == 0) {
            break;
        }
        slen += len;
    }
    if (slen > 0) {
        token_pack_string(TOKEN, slen, string); // may replace string with a shortstr elem
        return SUCCESS;
    }
    return FAIL;
}

/**
 * process single character tokens
 *
 * @param TOKEN context
 * @return the state of the character just read
 */
state_t token(TOKEN_t * TOKEN)
{
    return (TOKEN->insi = char2state[*++(TOKEN->in)]);
}
