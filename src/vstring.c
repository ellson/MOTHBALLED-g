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
    int len;
    elem_t *elem;

    frag = in++;
    len = 1;
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
                switch (*in) {
                    case '"':
                        TOKEN->quote_type = 0;
                        quote_state = 0;
                        len++;
                        in++;
                        goto done;
                    case '\\':
                        quote_state = 2;
                        break;
                    case '\n':
                        TOKEN->stat_lfcount++;
                        break;
                    case '\r':
                        TOKEN->stat_crcount++;
                        break;
                }
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
                switch (*in) {
                    case '>':
                        if (--TOKEN->quote_nest <= 0) {
                            TOKEN->quote_type = 0;
                            quote_state = 0;
                            len++;
                            in++;
                            goto done;
                        }
                        break;
                    case '<':
                        ++TOKEN->quote_nest;
                        break;
                    case '\n':
                        TOKEN->stat_lfcount++;
                        break;
                    case '\r':
                        TOKEN->stat_crcount++;
                        break;
                }
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
 * load LBE..RBE quoted VSTRING fragment(s)  
 *
 * Quoting format:
 *     {..{..\}..\{..\\..}..} 
 *
 * @param TOKEN context
 * @param vstring
 * @return length of vstring
 */
static int vstring_fragment_LBE(TOKEN_t * TOKEN, elem_t *vstring)
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
            case 0: // leading LBE
                TOKEN->quote_nest = 1;
                quote_state = 1;
                break;
            case 1: // inside quote
                switch (*in) {
                    case '}':
                        if (--TOKEN->quote_nest <= 0) {
                            TOKEN->quote_type = 0;
                            quote_state = 0;
                            len++;
                            in++;
                            goto done;
                        }
                        break;
                    case '{':
                        ++TOKEN->quote_nest;
                        break;
                    case '\\':
                        quote_state = 2;
                        break;
                    case '\n':
                        TOKEN->stat_lfcount++;
                        break;
                    case '\r':
                        TOKEN->stat_crcount++;
                        break;
                }
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
    elem = new_frag(LIST(), LBE, len, frag);
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
                switch (*in) {
                    case ')':
                        if (--TOKEN->quote_nest <= 0) {
                            TOKEN->quote_type = 0;
                            quote_state = 0;
                            len++;
                            in++;
                            goto done;
                        }
                        break;
                    case '(':
                        ++TOKEN->quote_nest;
                        break;
                    case '\\':
                        quote_state = 2;
                        break;
                    case '\n':
                        TOKEN->stat_lfcount++;
                        break;
                    case '\r':
                        TOKEN->stat_crcount++;
                        break;
                }
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
 * load LBR..RBR quoted VSTRING fragment(s)  
 *
 * Length in [ ]  followed by that number of characters
 *
 * Quoting format:
 *     [10]1234567890
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
    unsigned char c;
    int quote_state = TOKEN->quote_state;
    int quote_length = TOKEN->quote_length;
    int len = 0;
    elem_t *elem;

    frag = in;
    while (in != end) {
        switch (quote_state) {
            case 0: // leading LBR
                quote_state = 1;
                quote_length = 0;
                break;
            case 1: // inside [...]
                c = *in;
                if (c >= '0' && c <= '9') {  // end of nested block
                    quote_length = quote_length * 10 + (c - '0');
                }
                else if (c == ']') {
                    if (quote_length) {
                        quote_state = 2;
                    }
                    else {
                        quote_state = 0;
                        TOKEN->quote_type = 0;
                        len++;
                        in++;
                        goto done;
                    }
                }
                else {
                    token_error(TOKEN, "Invalid length", VSTRING);
                }
                break;
            case 2: 
                if (quote_length-- == 0) {
                    quote_state = 0;
                    TOKEN->quote_type = 0;
                    len++;
                    in++;
                    goto done;
                }
                switch (*in) {
                    case '\n':
                        TOKEN->stat_lfcount++;
                        break;
                    case '\r':
                        TOKEN->stat_crcount++;
                        break;
                }
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
    TOKEN->quote_length = quote_length;
    elem = new_frag(LIST(), LBR, len, frag);
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
        switch (TOKEN->quote_type) { // if continuing a quote
            case DQT:
                len = vstring_fragment_DQT(TOKEN, vstring);
                break;
            case LAN:
                len = vstring_fragment_LAN(TOKEN, vstring);
                break;
            case LBE:
                len = vstring_fragment_LBE(TOKEN, vstring);
                break;
            case LPN:
                len = vstring_fragment_LPN(TOKEN, vstring);
                break;
            case LBR:
                len = vstring_fragment_LBR(TOKEN, vstring);
                break;
            default: // else its a new quote
                switch (char2vstate[*(TOKEN->in)]) { // use VSTRING table for extended ABC set
                    case ABC:
                        len = vstring_fragment_ABC(TOKEN, vstring);
                        break;
                    case DQT:
                        TOKEN->quote_type = DQT;
                        len = vstring_fragment_DQT(TOKEN, vstring);
                        break;
                    case LAN:
                        TOKEN->quote_type = LAN;
                        len = vstring_fragment_LAN(TOKEN, vstring);
                        break;
                    case LBE:
                        TOKEN->quote_type = LBE;
                        len = vstring_fragment_LBE(TOKEN, vstring);
                        break;
                    case LPN:
                        TOKEN->quote_type = LPN;
                        len = vstring_fragment_LPN(TOKEN, vstring);
                        break;
                    case LBR:
                        TOKEN->quote_type = LBR;
                        len = vstring_fragment_LBR(TOKEN, vstring);
                        break;
                    case AST:
                        TOKEN->elem_has_ast = AST;
                        len = vstring_fragment_AST(TOKEN, vstring);
                        break;
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
