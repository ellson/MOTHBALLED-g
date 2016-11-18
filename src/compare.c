/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "thread.h"
#include "compare.h"

#define MAXNEST 20

#define DBG 1

typedef struct {
    elem_t *next[MAXNEST];
    unsigned char *cp;
    uint16_t nest;
    uint16_t len;
#ifdef DBG
    elem_t * current;
#endif
} iter_t;

static elem_t * upnext(iter_t *iter)
{
    elem_t *elem = NULL;
#ifdef DBG
    static unsigned char popmark[] = {'+'};
#else
    static unsigned char popmark[] = {'\0'};
#endif

    if (iter->nest) {
        iter->cp = popmark;
        iter->len = sizeof(popmark);
        elem = iter->next[--(iter->nest)];
        if (elem) {
#ifdef DBG
            iter->current = elem;
#endif
            iter->next[iter->nest] = elem->u.l.next;
        }
    }
    return elem;
}

static elem_t * next(iter_t *iter, elem_t *elem)
{
#ifdef DBG
    static unsigned char pushmark[] = {'-'};
#else
    static unsigned char pushmark[] = {'\0'};
#endif

    assert(iter->nest < MAXNEST);

    if(elem) {
#ifdef DBG
        iter->current = elem;
#endif
        switch (elem->type) {
        case FRAGELEM:
            iter->cp = elem->u.f.frag;
            iter->len = elem->len;
            elem = elem->u.f.next;
            break;
        case SHORTSTRELEM:
            iter->cp = elem->u.s.str;
            iter->len = elem->len;
            elem = NULL;
            break;
        case LISTELEM:
            iter->next[(iter->nest)++] = elem->u.l.next;
            elem = elem->u.l.first;
#ifdef DBG
            iter->current = elem;
#endif
            if ((elemtype_t)elem->type == FRAGELEM) {
                // to align with SHORTSTRELEM
                iter->cp = elem->u.f.frag;
                iter->len = elem->len;
                elem->u.f.next;
#ifdef DBG
                iter->current = elem;
#endif
            } else {
                iter->cp = pushmark;
                iter->len = sizeof(pushmark);
            }
            break;
        default:
            assert(0);
            break;
        }
    } else {
        elem = upnext(iter);
    }
    return elem;
}

/**
 * compare string value of lists: a and b
 *
 * - every traversal up or down through nested lists contibutes a single
 *   '\0' to the comparison.   This is to ensure that:  a  != (a)
 *
 * @param a
 * @param b
 * @return result of ASCII comparison: <0, 0, >0
 */
int compare (elem_t *a, elem_t *b)
{
    int rc;
    iter_t ai = { 0 };
    iter_t bi = { 0 };

    a = next(&ai, a);
    b = next(&bi, b);
    do {
        do { 
            ai.len--;
            bi.len--;
            rc = (*ai.cp++) - (*bi.cp++);
        } while (ai.len && bi.len && rc == 0);
        if (rc == 0) {
            // not a match while one is shorter than the other
            rc = ai.len - bi.len;
        }
        if (a && ai.len == 0) {
            a = next(&ai, a);
        }
        if (b && bi.len == 0) {
            b = next(&bi, b);
        }
    } while (ai.len && bi.len && rc == 0);
    return rc;
}

/**
 * match string value of lists: a and b
 * b may contain strings with trailing '*' which will
 * match any tail of the corresponding string in a
 *
 * - every traversal up or down through nested lists contibutes a single
 *   '\0' to the comparison.   This is to ensure that:  a  != (a)
 *
 * @param a
 * @param b (may contain '*')
 * @return result of ASCII comparison: <0, 0, >0
 */
int match (elem_t *a, elem_t *b)
{
    int rc;
    iter_t ai = { 0 };
    iter_t bi = { 0 };

    a = next(&ai, a);
    b = next(&bi, b);
    do {
        do { 
#ifdef DBG
fprintf(stderr,"1: %p:%d:%d.%d: \"%c\" %p:%d:%d.%d: \"%c\"\n",
        ai.current, ai.current->state, ai.current->type, ai.len, *ai.cp,
        bi.current, bi.current->state, bi.current->type, bi.len, *bi.cp);
#endif
            if (*bi.cp == '*') { 
                //  "x..." matches "*"  where ai.len >= bi.len
                rc = 0;
                break;
            } 
            ai.len--;
            bi.len--;
            rc = (*ai.cp++) - (*bi.cp++);
        } while (ai.len && bi.len && rc == 0);
        if (rc == 0) {
            if (*bi.cp == '*') {
                //  "x..." matches "*"  where ai.len >= bi.len
                //  also
                //  "x" matches "x*"    where ai.len == bi.len - 1
                a = upnext(&ai);
                b = upnext(&bi);
#ifdef DBG
fprintf(stderr,"+> %p:%d:%d.%d: \"%c\" %p:%d:%d.%d: \"%c\"\n",
        ai.current, ai.current->state, ai.current->type, ai.len, *ai.cp,
        bi.current, bi.current->state, bi.current->type, bi.len, *bi.cp);
#endif
            } else {
                // not a match while one is shorter than the other
                rc = ai.len - bi.len;
            }
        }
        if (a && ai.len == 0) {
            a = next(&ai, a);
        }
        if (b && bi.len == 0) {
            b = next(&bi, b);
        }
#ifdef DBG
fprintf(stderr,"-> %p:%d:%d.%d: \"%c\" %p:%d:%d.%d: \"%c\"\n",
        ai.current->state, ai.current->type, ai.len, *ai.cp,
        bi.current->state, bi.current->type, bi.len, *bi.cp);
#endif
    } while (ai.len && bi.len && rc == 0);
    return rc;
}
