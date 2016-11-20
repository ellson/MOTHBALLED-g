/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "thread.h"
#include "compare.h"

#define MAXNEST 20

typedef struct {
    elem_t *nextstack[MAXNEST];
    unsigned char *cp, *push, *split, *pop;
    uint16_t sp;
    uint16_t len;
} iter_t;

static void stepiter(iter_t *iter, elem_t *this)
{
    switch ((elemtype_t)this->type) {
    case FRAGELEM:
        iter->nextstack[iter->sp] = this->u.f.next;
        iter->cp = this->u.f.frag;
        iter->len = this->len;
        break;
    case SHORTSTRELEM:
        iter->nextstack[iter->sp] = NULL;
        iter->cp = this->u.s.str;
        iter->len = this->len;
        break;
    case LISTELEM:
        if ((elemtype_t)this->u.l.first->type == FRAGELEM) {
            // align with SHORTSTRELEM
            this = this->u.l.first;
            iter->nextstack[iter->sp] = this->u.f.next;
            iter->cp = this->u.f.frag;
            iter->len = this->len;
        } else {
            assert(iter->sp < MAXNEST);
            iter->nextstack[(iter->sp)++] = this->u.l.next;
            iter->nextstack[(iter->sp)] = this->u.l.first;
            iter->cp = iter->push;
            iter->len = 1;
        }
        break;
    default:
        assert(0);
        break;
    }
}

static void inititer(iter_t *iter, elem_t *elem, char *push_split_pop)
{
    static char *default_push_split_pop = "\0\0\0";

    assert(iter);
    assert(elem);
    assert((elemtype_t)elem->type == LISTELEM
        || (elemtype_t)elem->type == SHORTSTRELEM);

    if (push_split_pop) {
        assert(*(push_split_pop) && *(push_split_pop+1) && *(push_split_pop+2));
        iter->push = (unsigned char*)push_split_pop;
    } else {
        iter->push = (unsigned char*)default_push_split_pop;
    }
    iter->split=iter->push+1;
    iter->pop=iter->push+2;

    stepiter(iter, elem);
}

static void skipiter(iter_t *iter)
{
    elem_t *this;

    if (iter->sp) {
        this = iter->nextstack[--(iter->sp)];
        if (this) {
            iter->nextstack[(iter->sp)++] = this->u.l.next;
            iter->nextstack[(iter->sp)] = this->u.l.first;
            iter->cp = iter->split;
            iter->len = 1;
        } else {
            iter->cp = iter->pop;
            iter->len = 1;
        }
    } else {
        iter->cp = NULL;
        iter->len = 0;
    }
}

static void nextiter(iter_t *iter)
{
    elem_t *this = iter->nextstack[iter->sp];

    if (this) {
        stepiter(iter, this);
    } else {
        skipiter(iter);
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

    inititer(&ai, a, NULL);
    inititer(&bi, b, NULL);
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
            nextiter(&ai);
        }
        if (bi.len == 0) {
            nextiter(&bi);
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

    inititer(&ai, a, NULL);
    inititer(&bi, b, NULL);
    do {
        do { 
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
                skipiter(&ai);  // skip to next string 
                skipiter(&bi);  // skip to next pattern
            } else {
                // not a match while one is shorter than the other
                rc = ai.len - bi.len;
            }
        }
        if (ai.len == 0) {
            nextiter(&ai);
        }
        if (bi.len == 0) {
            nextiter(&bi);
        }
    } while (ai.len && bi.len && rc == 0);
    return rc;
}

/**
 * printlist string value of lists: a and b
 *
 * - every traversal up, across, or down, through nested lists contibutes a single
 *   'char' to the printed.   Typically: '(', ' ', or ')'
 *
 * @param a
 */
void printlist (elem_t *a)
{
    iter_t ai = { 0 };

    inititer(&ai, a, "( )");
    do {
        if (ai.len) {
            while (ai.len--) {
                putc(*ai.cp++, stdout);
            }
        } 
        nextiter(&ai);
    } while (ai.len || ai.sp);
    putc('\n', stdout);
}
