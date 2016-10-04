/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "inbuf.h"
#include "grammar.h"
#include "list.h"
#include "token.h"

/**
 * report an error during parsing with context info.
 *
 * @param TOKENS context
 * @param si parser state
 * @param si error message
 */
void je_token_error(TOKENS_t * TOKENS, state_t si, char *message)
{
    unsigned char *p, c;

    fprintf(stderr, "Error: %s ", message);
    print_len_frag(stderr, NAMEP(si));
    fprintf(stderr, "\n      in \"%s\" line: %ld just before: \"",
        TOKENS->filename, (TOKENS->stat_lfcount ?  TOKENS->stat_lfcount : TOKENS->stat_crcount) - TOKENS->linecount_at_start + 1);
    p = TOKENS->in;
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
 * @param TOKENS context
 * @return success/fail
 */
static success_t je_more_in(TOKENS_t * TOKENS)
{
    INBUFS_t *INBUFS = (INBUFS_t *)TOKENS;
    int size;

    if (INBUFS->inbuf) {        // if there is an existing active-inbuf
        if (TOKENS->in == &(INBUFS->inbuf->end_of_buf)) {    // if it is full
            if ((--(INBUFS->inbuf->refs)) == 0) {    // dereference active-inbuf
                free_inbuf(INBUFS, INBUFS->inbuf);    // free if no refs left (unlikely)
            }
            INBUFS->inbuf = new_inbuf(INBUFS);    // get new
            assert(INBUFS->inbuf);
            INBUFS->inbuf->refs = 1;    // add active-inbuf reference
            TOKENS->in = INBUFS->inbuf->buf;    // point to beginning of buffer
        }
    } else {        // no inbuf, implies just starting
        INBUFS->inbuf = new_inbuf(INBUFS);    // get new
        assert(INBUFS->inbuf);
        INBUFS->inbuf->refs = 1;    // add active-inbuf reference
        TOKENS->in = INBUFS->inbuf->buf;
    }

    if (TOKENS->file) {        // if there is an existing active input file
        if (TOKENS->insi == NLL && feof(TOKENS->file)) {    //    if it is at EOF
            //   Although the grammar doesn't care, I decided that it would
            //   be more user-friendly to check that we are not in a quote string
            //   whenever EOF occurs.
            if (TOKENS->in_quote) {
                je_token_error(TOKENS, NLL, "EOF in the middle of a quote string");
            }
// FIXME don't close stdin
// FIXME - stall for more more input   (inotify events ?)
            fclose(TOKENS->file);    // then close it and indicate no active input file
            TOKENS->file = NULL;
        }
    }
    if (!TOKENS->file) {        // if there is no active input file
        if (*(TOKENS->pargc) > 0) {    //   then try to open the next file
            TOKENS->filename = TOKENS->argv[0];
            (*(TOKENS->pargc))--;
            TOKENS->argv = &(TOKENS->argv[1]);
            if (strcmp(TOKENS->filename, "-") == 0) {
                TOKENS->file = stdin;
                *(TOKENS->pargc) = 0;    // No files after stdin
            } else {
                TOKENS->file = fopen(TOKENS->filename, "rb");
                if (!TOKENS->file) {
                    je_token_error(TOKENS, ACTIVITY, "fopen fail");
                }
            }
            TOKENS->linecount_at_start = TOKENS->stat_lfcount ? TOKENS->stat_lfcount : TOKENS->stat_crcount;
            TOKENS->stat_filecount++;
        } else {
            return FAIL;    // no more input available
        }
        assert(TOKENS->file);
    }
    // slurp in data from file stream
    size = fread(TOKENS->in, 1, &(INBUFS->inbuf->end_of_buf) - TOKENS->in, TOKENS->file);
    TOKENS->in[size] = '\0';    // ensure terminated (we have an extras
    //    character in inbuf_t for this )
    TOKENS->insi = char2state[*TOKENS->in];

    TOKENS->stat_inchars += size;
    return SUCCESS;
}

/**
 * consume comment fagments
 *
 * @param TOKENS context
 */
static void je_token_comment_fragment(TOKENS_t * TOKENS)
{
    unsigned char *in, c;

    in = TOKENS->in;
    c = *in;
    while (c != '\0' && c != '\n' && c != '\r') {
        c = *++in;
    }
    TOKENS->insi = char2state[c];
    TOKENS->in = in;
}

/**
 * consume all comment up to next token, or EOF
 *
 * @param TOKENS context
 * @return success/fail
 */
static success_t je_token_comment(TOKENS_t * TOKENS)
{
    success_t rc;

    rc = SUCCESS;
    je_token_comment_fragment(TOKENS);    // eat comment
    while (TOKENS->insi == NLL) {    // end_of_buffer, or EOF, during comment
        if ((rc = je_more_in(TOKENS) == FAIL)) {
            break;    // EOF
        }
        je_token_comment_fragment(TOKENS);    // eat comment
    }
    return rc;
}

/**
 * consume whitespace fagments
 *
 * @ TOKENS context
 */
static void je_token_whitespace_fragment(TOKENS_t * TOKENS)
{
    unsigned char *in, c;
    state_t insi;

    if ((in = TOKENS->in)) {
        c = *in;
        insi = TOKENS->insi;
        while (insi == WS) {    // eat all leading whitespace
            if (c == '\n') {
                TOKENS->stat_lfcount++;
            }
            if (c == '\r') {
                TOKENS->stat_crcount++;
            }
            c = *++in;
            insi = char2state[c];
        }
        TOKENS->insi = insi;
        TOKENS->in = in;
    }
}

/**
 * consume all non-comment whitespace up to next token, or EOF
 *
 * @param TOKENS context
 * @return success/fail
 */
static success_t je_token_non_comment(TOKENS_t * TOKENS)
{
    success_t rc;

    rc = SUCCESS;
    je_token_whitespace_fragment(TOKENS);    // eat whitespace
    while (TOKENS->insi == NLL) {    // end_of_buffer, or EOF, during whitespace
        if ((rc = je_more_in(TOKENS) == FAIL)) {
            break;    // EOF
        }
        je_token_whitespace_fragment(TOKENS);    // eat all remaining leading whitespace
    }
    return rc;
}

/**
 * consume all whitespace or comments up to next token, or EOF
 *
 * @param TOKENS context
 * @return success/fail
 */
success_t je_token_whitespace(TOKENS_t * TOKENS)
{
    success_t rc;

    rc = SUCCESS;
    while (1) {
        if ((rc = je_token_non_comment(TOKENS)) == FAIL) {
            break;
        }
        if (TOKENS->insi != OCT) {
            break;
        }
        while (TOKENS->insi == OCT) {
            if ((rc = je_token_comment(TOKENS)) == FAIL) {
                break;
            }
        }
    }
    return rc;
}

/**
 * load STRING fragments
 *
 * @param TOKENS context
 * @param fraglist - list of frags constituting a string
 * @return length of string
 */
static int je_token_string_fragment(TOKENS_t * TOKENS, elem_t * fraglist)
{
    LISTS_t *LISTS = (LISTS_t *)TOKENS;
    unsigned char *frag;
    state_t insi;
    int slen, len;
    elem_t *elem;

    slen = 0;
    while (1) {
        if (TOKENS->in_quote) {
            if (TOKENS->in_quote == 2) {    // character after BSL
                TOKENS->in_quote = 1;
                frag = TOKENS->in;
                TOKENS->insi = char2state[*++(TOKENS->in)];
                elem = new_frag(LISTS, BSL, 1, frag);
                slen++;
            } else if (TOKENS->insi == DQT) {
                TOKENS->in_quote = 0;
                TOKENS->insi = char2state[*++(TOKENS->in)];
                continue;
            } else if (TOKENS->insi == BSL) {
                TOKENS->in_quote = 2;
                TOKENS->insi = char2state[*++(TOKENS->in)];
                continue;
            } else if (TOKENS->insi == NLL) {
                break;
            } else {
                frag = TOKENS->in;
                len = 1;
                while (1) {
                    insi = char2state[*++(TOKENS->in)];
                    if (insi == DQT || insi == BSL || insi == NLL) {
                        break;
                    }
                    len++;
                }
                TOKENS->insi = insi;
                elem = new_frag(LISTS, DQT, len, frag);
                slen += len;
            }
        } else if (TOKENS->insi == ABC) {
            frag = TOKENS->in;
            len = 1;
            while ((insi = char2state[*++(TOKENS->in)]) == ABC) {
                len++;
            }
            TOKENS->insi = insi;
            elem = new_frag(LISTS, ABC, len, frag);
            slen += len;
        } else if (TOKENS->insi == AST) {
            TOKENS->has_ast = 1;
            frag = TOKENS->in;
            while ((TOKENS->insi = char2state[*++(TOKENS->in)]) == AST) {
            }    // extra '*' ignored
            elem = new_frag(LISTS, AST, 1, frag);
            slen++;
        } else if (TOKENS->insi == DQT) {
            TOKENS->in_quote = 1;
            TOKENS->has_quote = 1;
            TOKENS->insi = char2state[*++(TOKENS->in)];
            continue;
        } else {
            break;
        }
        append_list(fraglist, elem);
        TOKENS->stat_fragcount++;
    }
    return slen;
}

/**
 * collect fragments to form a STRING token
 *
 * @param TOKENS context
 * @param fraglist
 * @return success/fail
 */
 
success_t je_token_string(TOKENS_t * TOKENS, elem_t * fraglist)
{
    int len, slen;

    TOKENS->has_quote = 0;
    slen = je_token_string_fragment(TOKENS, fraglist);    // leading string
    while (TOKENS->insi == NLL) {    // end_of_buffer, or EOF, during whitespace
        if ((je_more_in(TOKENS) == FAIL)) {
            break;    // EOF
        }
        if ((len = je_token_string_fragment(TOKENS, fraglist)) == 0) {
            break;
        }
        slen += len;
    }
    if (slen > 0) {
        TOKENS->stat_stringcount++;
        if (TOKENS->has_quote) {
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
 * @param TOKENS context
 * @param fraglist
 * @return length of string
 */
static int je_token_vstring_fragment(TOKENS_t * TOKENS, elem_t * fraglist)
{
    LISTS_t *LISTS = (LISTS_t *)TOKENS;
    unsigned char *frag;
    state_t insi;
    int slen, len;
    elem_t *elem;

    slen = 0;
    while (1) {
        if (TOKENS->in_quote) {
            if (TOKENS->in_quote == 2) {    // character after BSL
                TOKENS->in_quote = 1;
                frag = TOKENS->in;
                TOKENS->insi = char2state[*++(TOKENS->in)];
                elem = new_frag(LISTS, BSL, 1, frag);
                slen++;
            } else if (TOKENS->insi == DQT) {
                TOKENS->in_quote = 0;
                TOKENS->insi = char2state[*++(TOKENS->in)];
                continue;
            } else if (TOKENS->insi == BSL) {
                TOKENS->in_quote = 2;
                TOKENS->insi = char2state[*++(TOKENS->in)];
                continue;
            } else if (TOKENS->insi == NLL) {
                break;
            } else {
                frag = TOKENS->in;
                len = 1;
                while (1) {
                    insi = char2state[*++(TOKENS->in)];
                    if (insi == DQT || insi == BSL || insi == NLL) {
                        break;
                    }
                    len++;
                }
                TOKENS->insi = insi;
                elem = new_frag(LISTS, DQT, len, frag);
                slen += len;
            }
        // In the unquoted portions of VSTRING we allow '/' '\' ':' '?'
        // in addition to the ABC class
        // this allows URIs as values without quoting
        } else if (TOKENS->insi == ABC ||
                   TOKENS->insi == FSL ||
                   TOKENS->insi == BSL ||
                   TOKENS->insi == CLN ||
                   TOKENS->insi == QRY) {
            frag = TOKENS->in;
            len = 1;
            while ((insi = char2state[*++(TOKENS->in)]) == ABC ||
                    insi == FSL ||
                    insi == BSL ||
                    insi == CLN ||
                    insi == QRY) {
                len++;
            }
            TOKENS->insi = insi;
            elem = new_frag(LISTS, ABC, len, frag);
            slen += len;

        // but '*' are still special  (maybe used ias wild card in queries)
        } else if (TOKENS->insi == AST) {
            TOKENS->has_ast = 1;
            frag = TOKENS->in;
            while ((TOKENS->insi = char2state[*++(TOKENS->in)]) == AST) {
            }    // extra '*' ignored
            elem = new_frag(LISTS, AST, 1, frag);
            slen++;
        } else if (TOKENS->insi == DQT) {
            TOKENS->in_quote = 1;
            TOKENS->has_quote = 1;
            TOKENS->insi = char2state[*++(TOKENS->in)];
            continue;
        } else {
            break;
        }
        append_list(fraglist, elem);
        TOKENS->stat_fragcount++;
    }
    return slen;
}

/**
 * collect fragments to form a VSTRING token
 *
 * @param TOKENS context
 * @param fraglist
 * @return success/fail
 */
success_t je_token_vstring(TOKENS_t * TOKENS, elem_t * fraglist)
{
    int len, slen;

    TOKENS->has_quote = 0;
    slen = je_token_vstring_fragment(TOKENS, fraglist);    // leading string
    while (TOKENS->insi == NLL) {    // end_of_buffer, or EOF, during whitespace
        if ((je_more_in(TOKENS) == FAIL)) {
            break;    // EOF
        }
        if ((len = je_token_vstring_fragment(TOKENS, fraglist)) == 0) {
            break;
        }
        slen += len;
    }
    if (slen > 0) {
        TOKENS->stat_stringcount++;     //FIXME ?
        if (TOKENS->has_quote) {
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
 * @param TOKENS context
 * @return success/fail
 */
success_t je_token(TOKENS_t * TOKENS)
{
    char token;

    token = state_token[TOKENS->insi];
    TOKENS->insi = char2state[*++(TOKENS->in)];
    return SUCCESS;
}
