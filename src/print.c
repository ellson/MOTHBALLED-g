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
#include <assert.h>

#include "thread.h"

/**
 * Print a single fragment of len contiguous characters.
 *
 * @param chan FILE for output
 * @param len number of contiguous characters to be output
 * @param frag pointer to the first character in the fragment
 */
static void print_one_frag(FILE * chan, unsigned int len, unsigned char *frag)
{
    while (len--) {
        putc(*frag++, chan);
    }
}

/**
 * Print a len_frag (an 8bit length, followed by that number of characters)
 *
 * @param chan FILE for output
 * @param len_frag an 8bit length, followed by that number of characters
 */
uint16_t print_len_frag(FILE * chan, unsigned char *len_frag)
{
    uint16_t len = *len_frag++;
    print_one_frag(chan, len, len_frag);
    return len;
}

/**
 * Conditionaly print a separator followed by the concatenation of
 * fragments in the list.
 * The composite string is quoted as necessary to comply with g syntax
 * (although not necessarily in the same way as in the original input).
 *
 * @param chan FILE for output
 * @param state an indicator if the string is to be quoted
 * @param elem the first frag of the fragllist
 * @param sep if not NULL then a character to be printed first
 */
static void print_frags(FILE * chan, state_t state, elem_t * elem, char *sep)
{
    assert(sep);
    if (*sep) {
        putc(*sep, chan);
    }
    if (state == DQT) {
        putc('"', chan);
    }
    while (elem) {
        assert(elem->type == FRAGELEM);
        unsigned char *frag = elem->u.f.frag;
        uint16_t len = elem->len;
        assert(len > 0);
        if ((state_t) elem->state == BSL) {
            putc('\\', chan);
        }
        if ((state_t) elem->state == AST) {
            if (state == DQT) {
                putc('"', chan);
                putc('*', chan);
                putc('"', chan);
            } else {
                putc('*', chan);
            }
        }
        else {
            print_one_frag(chan, len, frag);
        }
        elem = elem->u.f.next;
    }
    if (state == DQT) {
        putc('"', chan);
    }
    *sep = ' ';
}

/**
 * Print the string content of a SHORTSTR elem
 *
 * @param chan FILE for output
 * @param elem the shortstring elem
 * @param sep if not NULL then a character to be printed first
 */
static void print_shortstr(FILE * chan, elem_t *elem, char *sep)
{
    assert(elem->type == (char)SHORTSTRELEM);
    assert(sep);

    if (*sep) {
        putc(*sep, chan);
    }
    if (elem->state == DQT) {
        putc('"', chan);
    }
    print_one_frag(chan, elem->len, elem->u.s.str);
    if (elem->state == DQT) {
        putc('"', chan);
    }
    *sep = ' ';
}

static void print_tree(THREAD_t * THREAD, elem_t * p, int *cnt, int indent)
 {
    FILE *chan = IO()->out;
    char *sep = &(THREAD->sep);
    elem_t *key;
    int ind;

    if (p->u.t.left) {
        print_tree(THREAD, p->u.t.left, cnt, indent);
    }
 
    if ((*cnt)++) {
        putc('\n', chan);
        putc(' ', chan);
        if (indent >= 0) {
            ind = indent;
            while (ind--) {
                putc(' ', chan);
            }
        }
    } else {
        putc(' ', chan);
    }
    putc('+', chan);
    ind = indent + 2;
    key = p->u.t.key;
    switch (key->type) {
        case LISTELEM:
            print_elem(THREAD, p->u.t.key, ind);
            break;
        case FRAGELEM:
            assert(key->u.l.first);
            assert(key->u.l.first->u.l.first);
            print_frags(chan, 0, key->u.l.first->u.l.first, sep);
            break;
        case SHORTSTRELEM:
            print_shortstr(chan, key, sep);
            break;
        default:
            assert(0);
    }
 
    if (p->u.t.right) {
        print_tree(THREAD, p->u.t.right, cnt, indent);
    }
 }
 
/**
 * Print an elem, recursively, with appropriate separators and indentation
 *
 * If non-negative initial indent, each nested list is printed at an incremented indent
 *
 * @param THREAD context for i/o
 * @param elem to be printed
 * @param indent if not -ve, then the initial indent
 */
void print_elem(THREAD_t * THREAD, elem_t * elem, int indent)
{
    FILE *chan = IO()->out;
    char *sep = &(THREAD->sep);

    if (elem) {
        elemtype_t type = (elemtype_t)elem->type;
        int cnt, width, ind;
        switch (type) {
        case FRAGELEM:
            print_frags(chan, elem->state, elem, sep);
            break;
        case LISTELEM:
            for (cnt = 0, width = 0; elem; elem = elem->u.l.next) {
                assert((elemtype_t)elem->type == type);    // check all the same type
                if (cnt++) {
                    putc('\n', chan);
                    putc(' ', chan);
                    if (indent >= 0) {
                        ind = indent;
                        while (ind--) {
                            putc(' ', chan);
                        }
                    }
                } else {
                    putc(' ', chan);
                }
                width = print_len_frag(chan, NAMEP(elem->state));
                ind = indent + width + 1;
                print_elem(THREAD, elem->u.l.first, ind);    // recurse
            }
            break;
        case SHORTSTRELEM:
            putc(' ', chan);
            print_len_frag(chan, NAMEP(elem->state));
            print_shortstr(chan, elem, sep);
            break;
        case TREEELEM:
            cnt = 0;
            print_tree(THREAD, elem, &cnt, indent);
            break;
        }
    }
}

void append_token(THREAD_t * THREAD, char **pos, char tok)
{
    // FIXME - check available buffer space
                        // ignore sep before
    *(*pos)++ = (unsigned char)tok;    // copy token
    **pos = '\0';       // and replace terminating NULL
    THREAD->sep = 0;        // no sep required after tokens
}

void append_string(THREAD_t * THREAD, char **pos, char *string)
{
    int len;

    // FIXME - check available buffer space
    if (THREAD->sep) {
        *(*pos)++ = THREAD->sep; // sep before, if any
    }
    len = sprintf(*pos,"%s",string);  // copy string
    if (len < 0)
        FATAL("sprintf()");
    THREAD->sep = ' ';      // sep required after strings 
    *pos += len;
}

void append_ulong(THREAD_t * THREAD, char **pos, uint64_t integer)
{
    int len;

    // FIXME - check available buffer space
    if (THREAD->sep) {
        *(*pos)++ = THREAD->sep; // sep before, if any
    }
    len = sprintf(*pos,"%lu",(unsigned long)integer); // format integer to string
    if (len < 0)
        FATAL("sprintf()");
    THREAD->sep = ' ';      // sep required after strings
    *pos += len;
}

// special case formatter for runtime
void append_runtime(THREAD_t * THREAD, char **pos, uint64_t run_sec, uint64_t run_ns)
{
    int len;

    // FIXME - check available buffer space
    if (THREAD->sep) *(*pos)++ = THREAD->sep; // sep before, if any
    len = sprintf(*pos,"%lu.%09lu",(unsigned long)run_sec, (unsigned long)run_ns);
    if (len < 0)
        FATAL("sprintf()");
    THREAD->sep = ' ';   // sep required after strings
    *pos += len;
}

