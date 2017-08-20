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
    unsigned char *frag;
    int len = 0;
    elem_t *elem;

    frag = in++;
    while (in != end) {
        if (char2vstate[*in] != ABC) {
            break;
        }
        in++;
        len++;
    }
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

    frag = in++;
    while (in != end) {
        if (*in != '*') {
            break;
        }
        in++;
    }
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
    int quote_state = TOKEN->quote_state;
    int len = 0;
    elem_t *elem;

    frag = in;
    while (in != end) {
        switch (quote_state) {
            case 0: // leading quote
                quote_state = 1;
                break;
            case 1: // inside quote
                if (*in == '"') {  // end of quote
                    TOKEN->quote_type = 0;
                    quote_state = 0;
                    len++;
                    in++;
                    goto done;
                } else if (*in == '\\') {  // escape next character
                    quote_state = 2;
                }
                // else its a character within the quotes
                break;
            case 2: // escaped character
                quote_state = 1;
                break;
            default:
                FATAL("shouldn't happen");
        }
        len++;
        in++;
    }
done:
    TOKEN->in = in;
    TOKEN->quote_state = quote_state;
    elem = new_frag(LIST(), DQT, len, frag);
    append_transfer(vstring, elem);
    TOKEN->stat_infragcount++;
    return len;
}

/**
 * load LAN..RAN quoted VSTRING fragment(s)  
 *
 * Quoting format:
 *     <..<...>..> 
 *
 * @param TOKEN context
 * @param vstring
 * @return length of vstring
 */
static int vstring_fragment_LAN(TOKEN_t * TOKEN, elem_t *vstring)
{
    unsigned char *frag;
    unsigned char *in = TOKEN->in;
    unsigned char *end = TOKEN->end;
    int quote_state = TOKEN->quote_state;
    int len = 0;
    elem_t *elem;

    frag = in;
    while (in != end) {
        switch (quote_state) {
            case 0: // leading LAN
                TOKEN->quote_nest = 1;
                quote_state = 1;
                break;
            case 1: // inside quote
                if (*in == '>') {  // end of nested block
                    if (--TOKEN->quote_nest <= 0) {
                        TOKEN->quote_type = 0;
                        quote_state = 0;
                        len++;
                        in++;
                        goto done;
                    }
                    // else its the end of a nested block
                }
                else if (*in == '<') {  // beginning of nested block
                    ++TOKEN->quote_nest;
                }
                // else its a character within the quotes
                break;
            default:
                FATAL("shouldn't happen");
        }
        len++;
        in++;
    }
done:
    TOKEN->in = in;
    TOKEN->quote_state = quote_state;
    elem = new_frag(LIST(), LAN, len, frag);
    append_transfer(vstring, elem);
    TOKEN->stat_infragcount++;
    return len;
}

/**
 * load LBR..RBR quoted VSTRING fragment(s)  
 *
 * Quoting format:
 *     {..{..\}..\{..\\..}..} 
 *
 * @param TOKEN context
 * @param vstring
 * @return length of vstring
 */
static int vstring_fragment_LBR(TOKEN_t * TOKEN, elem_t *vstring)
{
    unsigned char *frag;
    unsigned char *in = TOKEN->in;
    unsigned char *end = TOKEN->end;
    int quote_state = TOKEN->quote_state;
    int len = 0;
    elem_t *elem;

    frag = in;
    while (in != end) {
        switch (quote_state) {
            case 0: // leading LBR
                TOKEN->quote_nest = 1;
                quote_state = 1;
                break;
            case 1: // inside quote
                if (*in == '}') {  // end of nested block
                    if (--TOKEN->quote_nest <= 0) {
                        TOKEN->quote_type = 0;
                        quote_state = 0;
                        len++;
                        in++;
                        goto done;
                    }
                    // else its the end of a nested block
                }
                else if (*in == '{') {  // beginning of nested block
                    ++TOKEN->quote_nest;
                }
                // else its a character within the quotes
                break;
            default:
                FATAL("shouldn't happen");
        }
        len++;
        in++;
    }
done:
    TOKEN->in = in;
    TOKEN->quote_state = quote_state;
    elem = new_frag(LIST(), LBR, len, frag);
    append_transfer(vstring, elem);
    TOKEN->stat_infragcount++;
    return len;
}

/**
 * load LPN..RPN quoted VSTRING fragment(s)  
 *
 * Quoting format:
 *     (..(..\)..\(..\\..)..) 
 *
 * @param TOKEN context
 * @param vstring
 * @return length of vstring
 */
static int vstring_fragment_LPN(TOKEN_t * TOKEN, elem_t *vstring)
{
    unsigned char *frag;
    unsigned char *in = TOKEN->in;
    unsigned char *end = TOKEN->end;
    int quote_state = TOKEN->quote_state;
    int len = 0;
    elem_t *elem;

    frag = in;
    while (in != end) {
        switch (quote_state) {
            case 0: // leading LPN
                TOKEN->quote_nest = 1;
                quote_state = 1;
                break;
            case 1: // inside quote
                if (*in == ')') {  // end of nested block
                    if (--TOKEN->quote_nest <= 0) {
                        TOKEN->quote_type = 0;
                        quote_state = 0;
                        len++;
                        in++;
                        goto done;
                    }
                    // else its the end of a nested block
                }
                else if (*in == '(') {  // beginning of nested block
                    ++TOKEN->quote_nest;
                } else if (*in == '\\') {  // escape next character
                    quote_state = 2;
                }
                // else its a character within the quotes
                break;
            case 2: // escaped character
                quote_state = 1;
                break;
            default:
                FATAL("shouldn't happen");
        }
        len++;
        in++;
    }
done:
    TOKEN->in = in;
    TOKEN->quote_state = quote_state;
    elem = new_frag(LIST(), LPN, len, frag);
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
    TOKEN->quote_state = 0;
    do {
        if (TOKEN->in == TOKEN->end) {
            if ((token_more_in(TOKEN) == FAIL)) {
                break;
            }
        }
        switch (TOKEN->quote_type) {
            case DQT:
                len = vstring_fragment_DQT(TOKEN, vstring);
                break;  // break for switch
            case LAN:
                len = vstring_fragment_LAN(TOKEN, vstring);
                break;  // break for switch
            case LBR:
                len = vstring_fragment_LBR(TOKEN, vstring);
                break;  // break for switch
            case LPN:
                len = vstring_fragment_LPN(TOKEN, vstring);
                break;  // break for switch
            default:
                switch (char2vstate[*(TOKEN->in)]) { // use VSTRING table for extended ABC set
                    case ABC:
                        len = vstring_fragment_ABC(TOKEN, vstring);
                        break;  // break from switch
                    case DQT:
                        TOKEN->quote_type = DQT;
                        len = vstring_fragment_DQT(TOKEN, vstring);
                        break;  // break from switch
                    case LAN:
                        TOKEN->quote_type = LAN;
                        len = vstring_fragment_LAN(TOKEN, vstring);
                        break;  // break from switch
                    case LBR:
                        TOKEN->quote_type = LBR;
                        len = vstring_fragment_LBR(TOKEN, vstring);
                        break;  // break from switch
                    case LPN:
                        TOKEN->quote_type = LPN;
                        len = vstring_fragment_LPN(TOKEN, vstring);
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

    if (TOKEN->in == TOKEN->end) {
        TOKEN->insi = END;
    }
    else {
        TOKEN->insi = char2state[*(TOKEN->in)];  // back to regular table
    }

    if (slen == 0) {
        return FAIL;
    }

    // maybe replace fraglist with a shortstr elem
    token_pack_string(TOKEN, slen, vstring);
    return SUCCESS;
}
