/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "thread.h"
#include "compare.h"

#define MAXNEST 5

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
            iter->cp = &(elem->u.s.str);
            iter->len = elem->len;
            iter->next[iter->nest] = NULL;
            break;
        case LISTELEM:
            iter->next[iter->nest++] = elem->u.l.next;
            init(iter, elem->u.l.first);
            break;
        default:
            assert(0);
            break;
    }
}

/**
 * compare string value of elems: A and B
 *
 * - root may be LISTELEM or SHORTSTRELEM
 * - leaves maybe SHORSTRELEM or FRAGELEM
 *
 * @param A 
 * @param B
 * @return result of ASCII comparison: <0, 0, >0
 */

int compare (elem_t *A, elem_t *B)
{
    unsigned char a, b;
    int rc;
    iter_t ai = { 0 };
    iter_t bi = { 0 };

    init(&ai, A);
    init(&bi, B);
    do {
        do { 
            a = *ai.cp++; ai.len--;
            b = *bi.cp++; bi.len--;
            rc = a - b;
        } while (ai.len && bi.len && !rc);
        if (! ai.len) {
            if (ai.next[ai.nest]) { 
                ai.cp = ai.next[ai.nest]->u.f.next;
                ai.len = ai.next[ai.nest]->len;
                ai.next[ai.nest] = ai.next[ai.nest]->u.f.next;
                a = *ai.cp;
            } else {
                while (--ai.nest && ! ai.next[ai.nest]) {}
                if (ai.nest) {
		    init(&ai, ai.next[ai.nest]->u.l.first);
		}
                a = '\0';
            }
        }
        if (! bi.len) {
            if (bi.next[bi.nest]) { 
                bi.cp = bi.next[bi.nest]->u.f.next;
                bi.len = bi.next[bi.nest]->len;
                bi.next[bi.nest] = bi.next[bi.nest]->u.f.next;
                b = *bi.cp;
            } else {
                while (--bi.nest && ! bi.next[bi.nest]) {}
                if (bi.nest) {
		    init(&bi, bi.next[bi.nest]->u.l.first);
		}
                b = '\0';
            }
        }
        rc = a - b;
    } while (ai.nest && bi.nest && !rc);
    return rc;
}
