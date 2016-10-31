/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "frag.h"

/**
 * Print a single fragment of len contiguous characters.
 *
 * @param chan output FILE*
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
 * @param chan output FILE*
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
 * @param chan output FILE*
 * @param state an indicator if the string is to be quoted
 * @param elem the first frag of the fragllist
 * @param sep if not NULL then a character to be printed first
 */
void print_frags(FILE * chan, state_t state, elem_t * elem, char *sep)
{
    unsigned char *frag;
    uint16_t len;

    assert(sep);
    if (*sep) {
        putc(*sep, chan);
    }
    if (state == DQT) {
        putc('"', chan);
    }
    while (elem) {

        assert(elem->type == FRAGELEM);

        frag = elem->u.f.frag;
        len = elem->len;
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

static void print_shortstr(FILE *chan, elem_t *elem, char *sep) {

    assert(elem->type == (char)SHORTSTRELEM);
    assert(sep);

    if (*sep) {
        putc(*sep, chan);
    }
    print_one_frag(chan, elem->len, elem->u.s.str);
}

/**
 * Print an elem, recursively, with appropriate separators and indentation
 *
 * If non-negative initial indent, each nested list is printed at an incremented indent
 *
 * @param chan output FILE*
 * @param elem to be printed
 * @param indent if not -ve, then the initial indent
 * @param sep if not NULL then a character to be printed first
 */
void print_elem(FILE * chan, elem_t * elem, int indent, char *sep)
{
    elemtype_t type;
    int ind, cnt, width;

    if (elem) {
        type = (elemtype_t) (elem->type);
        switch (type) {
        case FRAGELEM:
            print_frags(chan, elem->state, elem, sep);
            break;
        case LISTELEM:
            cnt = 0;
            width = 0;
            while (elem) {
                assert(elem->type == (char)type);    // check all the same type
                if (cnt++) {
                    putc('\n', chan);
                    putc(' ', chan);
                    if (indent >= 0) {
                        ind = indent;
                        while (ind--)
                            putc(' ', chan);
                    }
                } else {
                    putc(' ', chan);
                }
                width = print_len_frag(chan, NAMEP(elem->state));
                ind = indent + width + 1;
                print_elem(chan, elem->u.l.first, ind, sep);    // recurse
                elem = elem->u.l.next;
            }
            break;
        case SHORTSTRELEM:
            print_shortstr(chan, elem, sep);
            break;
        case TREEELEM:
            print_tree(chan, elem, sep);
            break;
        }
    }
}

/**
 * print a tree from left to right.  i.e in insertion sort order
 *
 * @param chan the FILE* for the output
 * @param p the root of the tree (should be in the middle of the resulting list)
 * @param sep the separator beween items
 */
void print_tree(FILE *chan, elem_t * p, char *sep)
{
    assert(p);
    assert(p->u.t.key);
    assert(p->u.t.key->u.l.first);
    assert(p->u.t.key->u.l.first->u.l.first);

    if (p->u.t.left) {
        print_tree(chan, p->u.t.left, sep);
    }
    print_frags(stdout, 0, p->u.t.key->u.l.first->u.l.first, sep);
    if (p->u.t.right) {
        print_tree(chan, p->u.t.right, sep);
    }
}

