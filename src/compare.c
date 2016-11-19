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
#ifdef DBG
    elem_t * this;
#endif
    unsigned char *cp;
    uint16_t sp;
    uint16_t len;
} iter_t;

static void step(iter_t *iter, elem_t *this)
{
    static unsigned char pushmark[] = {'\0'};

    switch ((elemtype_t)this->type) {
    case FRAGELEM:
        iter->next[iter->sp] = this->u.f.next;
        iter->cp = this->u.f.frag;
        iter->len = this->len;
        break;
    case SHORTSTRELEM:
        iter->next[iter->sp] = NULL;
        iter->cp = this->u.s.str;
        iter->len = this->len;
        break;
    case LISTELEM:
        this = this->u.l.first;
        if ((elemtype_t)this->type == FRAGELEM) {
            // to align with SHORTSTRELEM
            iter->next[iter->sp] = this->u.f.next;
            iter->cp = this->u.f.frag;
            iter->len = this->len;
        } else {
            assert(iter->sp < MAXNEST);
            iter->next[(iter->sp)++] = this->u.l.next;
            iter->next[(iter->sp)] = this->u.f.next;
            iter->cp = pushmark;
            iter->len = sizeof(pushmark);
        }
        break;
    default:
        assert(0);
        break;
    }
#ifdef DBG
    iter->this = this;
#endif
}

static void init(iter_t *iter, elem_t *elem)
{
    assert(iter);
    assert(elem);
    assert((elemtype_t)elem->type == LISTELEM
        || (elemtype_t)elem->type == SHORTSTRELEM);

    iter->sp = 0;
    step(iter, elem);
}

static void skip(iter_t *iter)
{
    static unsigned char popmark[] = {'\0'};
    elem_t *this;

    assert(iter->sp);
    this = iter->next[--(iter->sp)];
    if (this) {
        iter->next[(iter->sp)] = this->u.l.next;
        iter->cp = popmark;
        iter->len = sizeof(popmark);
    } else {
        iter->len = 0;
    }
#ifdef DBG
    iter->this = this;
#endif
}

static void next(iter_t *iter)
{
    elem_t *this;

    this = iter->next[iter->sp];
    if (this) {
        step(iter, this);
    } else {
        skip(iter);
    }
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

    init(&ai, a);
    init(&bi, b);
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
        if (ai.len == 0) {
            next(&ai);
        }
        if (bi.len == 0) {
            next(&bi);
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

    init(&ai, a);
    init(&bi, b);
    do {
        do { 
#ifdef DBG
fprintf(stderr,"--: %p:%d:%d.%d: \"%c\" %p:%d:%d.%d: \"%c\"\n",
        ai.this, ai.this->state, ai.this->type, ai.len, *ai.cp,
        bi.this, bi.this->state, bi.this->type, bi.len, *bi.cp);
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
                skip(&ai);  // skip to next string 
                skip(&bi);  // skip to next pattern
#ifdef DBG
fprintf(stderr,"+>: %p:%d:%d.%d: \"%c\" %p:%d:%d.%d: \"%c\"\n",
        ai.this, ai.this->state, ai.this->type, ai.len, *ai.cp,
        bi.this, bi.this->state, bi.this->type, bi.len, *bi.cp);
#endif
            } else {
                // not a match while one is shorter than the other
                rc = ai.len - bi.len;
            }
        }
        if (ai.len == 0) {
            next(&ai);
        }
        if (bi.len == 0) {
            next(&bi);
        }
#ifdef DBG
fprintf(stderr,"->: %p:%d:%d.%d: \"%c\" %p:%d:%d.%d: \"%c\"\n",
        ai.this, ai.this->state, ai.this->type, ai.len, *ai.cp,
        bi.this, bi.this->state, bi.this->type, bi.len, *bi.cp);
#endif
    } while (ai.len && bi.len && rc == 0);
    return rc;
}
