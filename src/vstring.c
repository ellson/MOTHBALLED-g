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
 * load VSTRING ABC fragments
 *
 * @param TOKEN context
 * @param vstring - list of frags constituting a vstring
 * @return length of vstring
 */
static int vstring_fragment_ABC(TOKEN_t * TOKEN, elem_t * vstring)
{
    unsigned char *in = TOKEN->in;
    unsigned char *end = TOKEN->end;
    state_t insi;
    unsigned char *frag;
    int len = 0;
    elem_t *elem;

    frag = TOKEN->in;
    while (in != end) {
        insi = char2vstate[*in];
        if (insi != ABC) {
            TOKEN->insi = insi;
            goto done;
        }
        in++;
        len++;
    }
    TOKEN->insi = END;
done:
    TOKEN->in = in;
    elem = new_frag(LIST(), ABC, len, frag);
    append_transfer(vstring, elem);
    TOKEN->stat_infragcount++;
    return len;
}

/**
 * load VSTRING AST fragments
 *
 * @param TOKEN context
 * @param vstring - list of frags constituting a vstring
 * @return length of vstring
 */
static int vstring_fragment_AST(TOKEN_t * TOKEN, elem_t * vstring)
{
    unsigned char *in = TOKEN->in;
    unsigned char *end = TOKEN->end;
    unsigned char *frag;
    elem_t *elem;

    frag = TOKEN->in;
    while (in != end) {
        if (*in != '*') {
            TOKEN->insi = char2vstate[*in];
            goto done;
        }
        in++;
    }
    TOKEN->insi = END;
done:
    TOKEN->in = in;
    elem = new_frag(LIST(), AST, 1, frag);
    append_transfer(vstring, elem);
    TOKEN->stat_infragcount++;
    return 1; // only interested in first AST
}

/**
 * load DQT quoted VSTRING fragment(s)
 *
 * Quoting format:
 *     "...\"...\\..." 
 *
 * @param TOKEN context
 * @param vstring
 * @return length of vstring
 */
static int vstring_fragment_DQT(TOKEN_t * TOKEN, elem_t *vstring)
{
    unsigned char *frag;
    unsigned char *in = TOKEN->in;
    unsigned char *end = TOKEN->end;
    int in_quote = TOKEN->in_quote;
    int len = 0;
    elem_t *elem;

    frag = in;
    while (in != end) {
        switch (in_quote) {
            case 0: // leading quote
                in_quote = 1;
                break;
            case 1: // inside quote
                if (*in == '"') {  // end of quote
                    TOKEN->quote_type = 0;
                    in_quote = 0;
                    len++;
                    in++;
                    goto done;
                } else if (*in == '\\') {  // escape next character
                    in_quote = 2;
                }
                // else its a character within the quotes
                break;
            case 2: // escaped character
                in_quote = 1;
                break;
            default:
                FATAL("shouldn't happen");
        }
        len++;
        in++;
    }
done:
    if (in == end) {
        TOKEN->insi = END;
    }
    else {
        TOKEN->insi = char2vstate[*in];
    }
    TOKEN->in = in;
    TOKEN->in_quote = in_quote;
    elem = new_frag(LIST(), DQT, len, frag);
    append_transfer(vstring, elem);
    TOKEN->stat_infragcount++;
    return len;
}

/**
 * collect fragments to form an VSTRING token
 *
 * @param TOKEN context
 * @param vstring
 * @return success/fail
 */
success_t token_vstring(TOKEN_t * TOKEN, elem_t *vstring)
{
    int len, slen=0;

    assert(vstring);
    assert(vstring->type == (char)LISTELEM);
    assert(vstring->refs > 0);

    TOKEN->quote_type = 0;
    TOKEN->in_quote = 0;
    do {
        switch (TOKEN->quote_type) {
            case DQT:
                len = vstring_fragment_DQT(TOKEN, vstring);
                break;  // break for switch
            default:
                switch (TOKEN->insi) {
                    case END:
                        if ((token_more_in(TOKEN) == FAIL)) {
                            len = 0; // EOF
                        }
                        continue;  // continue while
                    case ABC:
                        len = vstring_fragment_ABC(TOKEN, vstring);
                        break;  // break from switch
                    case DQT:
                        TOKEN->quote_type = DQT;
                        len = vstring_fragment_DQT(TOKEN, vstring);
                        break;  // break from switch
                    case AST:
                        TOKEN->elem_has_ast = AST;
                        len = vstring_fragment_AST(TOKEN, vstring);
                        break;  // break from switch
                    default:
                        len = 0;
                }
        }
        slen += len;
    } while (len);

    if (slen > 0) {
        // maybe replace fraglist with a shortstr elem
        token_pack_string(TOKEN, slen, vstring);
        return SUCCESS;
    }
    return FAIL;
}
