/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#include "token.h"

/**
 * report an error during parsing with context info.
 *
 * @param TOKEN context
 * @param si parser state
 * @param message error description
 */
void token_error(TOKEN_t * TOKEN, state_t si, char *message)
{
    unsigned char *p, c;

    fprintf(stderr, "Error: %s ", message);
    print_len_frag(stderr, NAMEP(si));
    fprintf(stderr, "\n      in \"%s\" line: %ld just before: \"",
        TOKEN->filename, (TOKEN->stat_lfcount ?  TOKEN->stat_lfcount : TOKEN->stat_crcount) - TOKEN->linecount_at_start + 1);
    p = TOKEN->in;
    while ((c = *p++)) {
        if (c == '\n' || c == '\r') {
            break;
        }
        putc(c, stderr);
    }
    fprintf(stderr, "\"\n");
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
                token_error(TOKEN, NLL, "EOF in the middle of a quote string");
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
                TOKEN->file = fopen(TOKEN->filename, "rb");
                if (!TOKEN->file) {
                    fatal_perror("Error - fopen()");
                }
            }
            TOKEN->linecount_at_start = TOKEN->stat_lfcount ? TOKEN->stat_lfcount : TOKEN->stat_crcount;
            TOKEN->stat_filecount++;
        } else {
            return FAIL;    // no more input available
        }
        assert(TOKEN->file);
    }
    // slurp in data from file stream
    size = fread(TOKEN->in, 1, &(INBUF->inbuf->end_of_buf) - TOKEN->in, TOKEN->file);
    TOKEN->in[size] = '\0';    // ensure terminated (we have an extras
    //    character in inbuf_t for this )
    TOKEN->insi = char2state[*TOKEN->in];

    TOKEN->stat_inchars += size;
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
    LIST_t *LIST = (LIST_t *)TOKEN;
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
                elem = new_frag(LIST, BSL, 1, frag);
                slen++;
            } else if (TOKEN->insi == DQT) {
                TOKEN->in_quote = 0;
                TOKEN->insi = char2state[*++(TOKEN->in)];
                continue;
            } else if (TOKEN->insi == BSL) {
                TOKEN->in_quote = 2;
                TOKEN->insi = char2state[*++(TOKEN->in)];
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
                elem = new_frag(LIST, DQT, len, frag);
                slen += len;
            }
        } else if (TOKEN->insi == ABC) {
            frag = TOKEN->in;
            len = 1;
            while ((insi = char2state[*++(TOKEN->in)]) == ABC) {
                len++;
            }
            TOKEN->insi = insi;
            elem = new_frag(LIST, ABC, len, frag);
            slen += len;
        } else if (TOKEN->insi == AST) {
            TOKEN->has_ast = 1;
            frag = TOKEN->in;
            while ((TOKEN->insi = char2state[*++(TOKEN->in)]) == AST) {
            }    // extra '*' ignored
            elem = new_frag(LIST, AST, 1, frag);
            slen++;
        } else if (TOKEN->insi == DQT) {
            TOKEN->in_quote = 1;
            TOKEN->has_quote = 1;
            TOKEN->insi = char2state[*++(TOKEN->in)];
            continue;
        } else {
            break;
        }
        append_list(fraglist, elem);
        TOKEN->stat_fragcount++;
    }
    return slen;
}

/**
 * collect fragments to form a STRING token
 *
 * @param TOKEN context
 * @param fraglist
 * @return success/fail
 */
 
success_t token_string(TOKEN_t * TOKEN, elem_t * fraglist)
{
    int len, slen;

    TOKEN->has_quote = 0;
    slen = token_string_fragment(TOKEN, fraglist);    // leading string
    while (TOKEN->insi == NLL) {    // end_of_buffer, or EOF, during whitespace
        if ((token_more_in(TOKEN) == FAIL)) {
            break;    // EOF
        }
        if ((len = token_string_fragment(TOKEN, fraglist)) == 0) {
            break;
        }
        slen += len;
    }
    if (slen > 0) {
        TOKEN->stat_stringcount++;
        if (TOKEN->has_quote) {
            fraglist->state = DQT;
        } else {
            fraglist->state = ABC;
        }
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
static int token_vstring_fragment(TOKEN_t * TOKEN, elem_t * fraglist)
{
    LIST_t *LIST = (LIST_t *)TOKEN;
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
                elem = new_frag(LIST, BSL, 1, frag);
                slen++;
            } else if (TOKEN->insi == DQT) {
                TOKEN->in_quote = 0;
                TOKEN->insi = char2state[*++(TOKEN->in)];
                continue;
            } else if (TOKEN->insi == BSL) {
                TOKEN->in_quote = 2;
                TOKEN->insi = char2state[*++(TOKEN->in)];
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
                elem = new_frag(LIST, DQT, len, frag);
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
            elem = new_frag(LIST, ABC, len, frag);
            slen += len;

        // but '*' are still special  (maybe used ias wild card in queries)
        } else if (TOKEN->insi == AST) {
            TOKEN->has_ast = 1;
            frag = TOKEN->in;
            while ((TOKEN->insi = char2state[*++(TOKEN->in)]) == AST) {
            }    // extra '*' ignored
            elem = new_frag(LIST, AST, 1, frag);
            slen++;
        } else if (TOKEN->insi == DQT) {
            TOKEN->in_quote = 1;
            TOKEN->has_quote = 1;
            TOKEN->insi = char2state[*++(TOKEN->in)];
            continue;
        } else {
            break;
        }
        append_list(fraglist, elem);
        TOKEN->stat_fragcount++;
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
success_t token_vstring(TOKEN_t * TOKEN, elem_t * fraglist)
{
    int len, slen;

    TOKEN->has_quote = 0;
    slen = token_vstring_fragment(TOKEN, fraglist);    // leading string
    while (TOKEN->insi == NLL) {    // end_of_buffer, or EOF, during whitespace
        if ((token_more_in(TOKEN) == FAIL)) {
            break;    // EOF
        }
        if ((len = token_vstring_fragment(TOKEN, fraglist)) == 0) {
            break;
        }
        slen += len;
    }
    if (slen > 0) {
        TOKEN->stat_stringcount++;     //FIXME ?
        if (TOKEN->has_quote) {
            fraglist->state = DQT;
        } else {
            fraglist->state = ABC;
        }
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
