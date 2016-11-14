/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "thread.h"
#include "compare.h"

#define MAXNEST 20

typedef struct {
    elem_t *next[MAXNEST];
    int nest;
    unsigned char *cp;
    int len;
} iter_t;


static void init(iter_t *iter, elem_t *elem)
{
    assert(iter->nest < MAXNEST);
    switch (elem->type) {
        case FRAGELEM:
            iter->cp = elem->u.f.frag;
            iter->len = elem->len;
            iter->next[iter->nest] = elem->u.f.next;
            break;
        case SHORTSTRELEM:
            iter->cp = elem->u.s.str;
            iter->len = elem->len;
            iter->next[iter->nest] = NULL;
            break;
        case LISTELEM:
            iter->next[iter->nest++] = NULL;  // next is the value, dont compare
            init(iter, elem->u.l.first);
            break;
        default:
            assert(0);
            break;
    }
}

/**
 * compare string value of elems: a and b
 *
 * - root may be LISTELEM or SHORTSTRELEM
 * - leaves maybe SHORSTRELEM or FRAGELEM
 *
 * @param a
 * @param b
 * @return result of ASCII comparison: <0, 0, >0
 */

int compare (elem_t *a, elem_t *b)
{
    unsigned char ac, bc;
    int rc;
    iter_t ai = { 0 };
    iter_t bi = { 0 };

    init(&ai, a);
    init(&bi, b);
    do {
        do { 
            ac = *ai.cp++; ai.len--;
            bc = *bi.cp++; bi.len--;
            rc = ac - bc;
        } while (ai.len && bi.len && rc == 0);
        if (! ai.len) {
            if (ai.next[ai.nest]) { 
                ai.cp = ai.next[ai.nest]->u.f.frag;
                ai.len = ai.next[ai.nest]->len;
                ai.next[ai.nest] = ai.next[ai.nest]->u.f.next;
                ac = *ai.cp;
            } else {
                if (ai.nest) {
                    while (--ai.nest && ! ai.next[ai.nest]) {}
                    if (! ai.nest) {
                        break;
                    }
		    init(&ai, ai.next[ai.nest]->u.l.first);
		}
                ac = '\0';
            }
        }
        if (! bi.len) {
            if (bi.next[bi.nest]) { 
                bi.cp = bi.next[bi.nest]->u.f.frag;
                bi.len = bi.next[bi.nest]->len;
                bi.next[bi.nest] = bi.next[bi.nest]->u.f.next;
                bc = *bi.cp;
            } else {
                if (bi.nest) {
                    while (--bi.nest && ! bi.next[bi.nest]) {}
                    if (!bi.nest) {
                        break;
                    }
		    init(&bi, bi.next[bi.nest]->u.l.first);
		}
                bc = '\0';
            }
        }
        if (rc == 0) {
            rc = ac - bc;
        }
    } while (ai.nest && bi.nest && rc == 0);
    return rc;
}

/**
 * match string value of elems: a and b_wild
 * b_wild may contain strings with trailing '*' which will
 * match any tail of the corresponding string in a
 *
 * - root may be LISTELEM or SHORTSTRELEM
 * - leaves maybe SHORSTRELEM or FRAGELEM
 *
 * @param a
 * @param b_wild
 * @return result of ASCII comparison: <0, 0, >0
 */

int match (elem_t *a, elem_t *b_wild)
{
    unsigned char ac, bc;
    int rc;
    iter_t ai = { 0 };
    iter_t bi = { 0 };

    init(&ai, a);
    init(&bi, b_wild);
    do {
        do { 
            if (*bi.cp == '*') {
                break;
            } 
            ac = *ai.cp++; ai.len--;
            bc = *bi.cp++; bi.len--;
            rc = ac - bc;
        } while (ai.len && bi.len && rc == 0);
        if (bi.len >= 0 && *bi.cp == '*') {
            bi.len=0;
        }
        if (! ai.len) {
            if (ai.next[ai.nest]) { 
                ai.cp = ai.next[ai.nest]->u.f.frag;
                ai.len = ai.next[ai.nest]->len;
                ai.next[ai.nest] = ai.next[ai.nest]->u.f.next;
                ac = *ai.cp;
            } else {
                if (ai.nest) {
                    while (--ai.nest && ! ai.next[ai.nest]) {}
                    if (! ai.nest) {
                        break;
                    }
		    init(&ai, ai.next[ai.nest]->u.l.first);
		}
                ac = '\0';
            }
        }
        if (! bi.len) {
            if (bi.next[bi.nest] && *bi.cp != '*') { 
                bi.cp = bi.next[bi.nest]->u.f.frag;
                bi.len = bi.next[bi.nest]->len;
                bi.next[bi.nest] = bi.next[bi.nest]->u.f.next;
                bc = *bi.cp;
            } else {
                if (bi.nest) {
                    while (--bi.nest && ! bi.next[bi.nest]) {}
                    if (!bi.nest) {
                        break;
                    }
		    init(&bi, bi.next[bi.nest]->u.l.first);
		}
                bc = '\0';
            }
        }
        if (rc == 0) {
            rc = ac - bc;
        }
    } while (ai.nest && bi.nest && rc == 0);
    return rc;
}
