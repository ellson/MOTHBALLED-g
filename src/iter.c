/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "types.h"
#include "inbuf.h"
#include "list.h"
#include "grammar.h"
#include "thread.h"
#include "iter.h"

static void begsep(iter_t *iter)
{
    char *cp = iter->lnxstack[iter->lsp].begsep;
    if (cp) {
        iter->len = strlen(cp);
        iter->cp = (unsigned char*)cp;
    }
    else {
        iter->len = 0;  // suppress nulls when sep char not required
        iter->cp = NULL;
    }
}

static void endsep(iter_t *iter)
{
    char *cp = iter->lnxstack[iter->lsp].endsep;
    if (cp) {
        iter->len = strlen(cp);
        iter->cp = (unsigned char*)cp;
    }
    else {
        iter->len = 0;  // suppress nulls when sep char not required
        iter->cp = NULL;
    }
}

static void nosep(iter_t *iter)
{
    iter->len = 0;
    iter->cp = NULL;
}

static void setsep(iter_t *iter, char *beg, char *end)
{
    iter->lnxstack[iter->lsp].begsep = beg;
    iter->lnxstack[iter->lsp].endsep = end;
}

/**
 * complete the iter state update for the new elem
 *
 * - Traversals up, down, or across lists may contribute
 *   an extra character to be printed that depends on the state_t
 *   e.g. The '<' '>' that surround edges.
 *
 * @param iter - a struct containg the current state of the iterator
 * @param this - the elem for this latest step
 */
static void stepiter(iter_t *iter, elem_t *this)
{
    elem_t *next, *first;

    switch ((elemtype_t)this->type) {
    case FRAGELEM:
        iter->lnxstack[iter->lsp].lnx = this->u.f.next;
        iter->cp = this->u.f.frag;
        iter->len = this->len;
        break;
    case SHORTSTRELEM:
        iter->lnxstack[iter->lsp].lnx = NULL;
        iter->cp = this->u.s.str;
        iter->len = this->len;
        break;
    case LISTELEM:
        if (this->u.l.first && (elemtype_t)this->u.l.first->type == FRAGELEM) {
            // align with SHORTSTRELEM
            this = this->u.l.first;
            iter->lnxstack[iter->lsp].lnx = this->u.f.next;
            iter->cp = this->u.f.frag;
            iter->len = this->len;
        } else {
            assert(iter->lsp < MAXNEST);
            switch ((state_t)this->state) {
                case ACT:
                    setsep(iter, NULL, NULL);
                    break;
                case EDGE:
                    setsep(iter, "<", ">");
                    break;
                case MUM:
                    setsep(iter, "^", NULL);
                    break;
                case SET:
                case ENDPOINTSET:
                    setsep(iter, "(", ")");
                    break;
                case ATTRID:
                    // FIXME - This is a hack! Probably the whole
                    //    psp spacing character scheme needs to be rethunk.
                    if (iter->intree) {
                        setsep(iter, " ", NULL);
                    }
                    else {
                        // suppress extra space before the attr=value list..
                        setsep(iter, NULL, NULL);
                    }
                    break;
                default:
                    setsep(iter, NULL, NULL);
                    break;
            }
            begsep(iter);
            iter->lnxstack[iter->lsp++].lnx = this->u.l.next;
            iter->lnxstack[iter->lsp].lnx = this->u.l.first;
        }
        break;
    case TREEELEM:
        if (iter->tsp == 0) {
            iter->tsp++;
            iter->len = 0;
            iter->tnxstack[iter->tsp].tnx = this;
            iter->tnxstack[iter->tsp].dir = 0;
            iter->intree = 0; // used to suppress extra space before the attr=value list..
        } else {
            iter->intree = 1;
        }
        do {
            if  (iter->tnxstack[iter->tsp].dir == 0) {
                iter->tnxstack[iter->tsp].dir++;
                next = iter->tnxstack[iter->tsp].tnx;
                if (next->u.t.left) {
                    iter->tsp++;
                    assert (iter->tsp < MAXNEST);
                    iter->tnxstack[iter->tsp].tnx = next->u.t.left;
                    iter->tnxstack[iter->tsp].dir = 0;
                    continue;
                }
            }
            if  (iter->tnxstack[iter->tsp].dir == 1) {
                next = iter->tnxstack[iter->tsp].tnx;
                iter->tnxstack[iter->tsp].dir++;
                first = next->u.t.key;
                break;
            }
            if (iter->tnxstack[iter->tsp].dir == 2) {
                iter->tnxstack[iter->tsp].dir++;
                next = iter->tnxstack[iter->tsp].tnx;
                if (next->u.t.right) {
                    iter->tsp++;
                    assert (iter->tsp < MAXNEST);
                    iter->tnxstack[iter->tsp].tnx = next->u.t.right;
                    iter->tnxstack[iter->tsp].dir = 0;
                    continue;
                }
            }
            iter->tsp--;
            if (iter->tsp == 0) {
                next = NULL;
                first = NULL;
                break;
            }
        } while (1); 
        iter->lnxstack[iter->lsp++].lnx = next;
        iter->lnxstack[iter->lsp].lnx = first;
        break;
    default:
        assert(0);
        break;
    }
}

/**
 * skip the iterator to the end of a chain of elems
 *    and to the next elem in traversal order
 *
 * @param iter - a struct containing the current state of the iterator
 */
static void skipiter(iter_t *iter)
{
    elem_t *this;

    if (iter->lsp) {
        this = iter->lnxstack[--iter->lsp].lnx;
        if (this) {
            switch ((elemtype_t)this->type) {
                case TREEELEM:
                    iter->cp = NULL;
                    iter->len = 0;
                    break;
                default:
                    switch ((state_t)this->state) {
                        // elems that follow elems of a different state_t
                        // (non-homogenous lists) need to over-ride the
                        // pop_space_push of the preceding elem
                        case ATTRIBUTES:
                            setsep(iter, "[", "]");
                            break;
                        case DISAMBIG:
                            setsep(iter, "'", NULL);
                            break;
                        case VALUE:
                            setsep(iter, "=", NULL);
                            break;
//                        case SIS:
//                            setsep(iter, " ", NULL);
//                            break;
                        case KID:
                            setsep(iter, "/", NULL);
                            break;
                        default:
                            setsep(iter, " ", NULL);
                            break;
                    }
                    begsep(iter);
                    iter->lnxstack[iter->lsp++].lnx = this->u.l.next;
                    iter->lnxstack[iter->lsp].lnx = this->u.l.first;
                    break;
            }
        } else {
            endsep(iter);
        }
    } else {
        nosep(iter);
    }
}

/**
 * initialize an iterator for traversing elem, its progeny, and its siblings
 *
 * @param iter - a struct containg the current state of the iterator
 * @param elem - the root elem of the list to be iterated
 */
static void inititer(iter_t *iter, elem_t *elem)
{
    assert(iter);
    assert(elem);
    assert((elemtype_t)elem->type == LISTELEM
        || (elemtype_t)elem->type == SHORTSTRELEM);
    iter->lsp = 0;
    stepiter(iter, elem);
}

/**
 * initialize an iterator for traversing elem and its progeny only (no siblings)
 *
 * @param iter - a struct containing the current state of the iterator
 * @param elem - the root elem of the list to be iterated
 */
static void inititer_no_siblings(iter_t *iter, elem_t *elem)
{
    assert(iter);
    assert(elem);
    assert((elemtype_t)elem->type == LISTELEM
        || (elemtype_t)elem->type == SHORTSTRELEM);
    iter->lsp = 0;
    stepiter(iter, elem);
    iter->lnxstack[0].lnx = NULL;
}

/**
 * move the iterator to the next elem in traversal order
 *
 * @param iter - a struct containing the current state of the iterator
 */
static void nextiter(iter_t *iter)
{
    elem_t *this = iter->lnxstack[iter->lsp].lnx;

    if (this) {
        stepiter(iter, this);
    } else {
        skipiter(iter);
    }
}

/**
 * compare string value of lists: a and b (including sublists, but not sibling lists)
 *
 * @param a
 * @param b
 * @return result of ASCII comparison: <0, 0, >0
 */
int compare (elem_t *a, elem_t *b)
{
    int rc = 0;
    iter_t ai = { 0 };
    iter_t bi = { 0 };

    inititer_no_siblings(&ai, a);  // compare a and a's progeny
    inititer_no_siblings(&bi, b);  //    with b and b's progeny
    do {
        while (ai.len && bi.len && rc == 0) {  // sep may be zero length
            ai.len--;
            bi.len--;
            rc = (*ai.cp++) - (*bi.cp++);
        }
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
    } while (rc == 0 && (ai.lsp || ai.len) && (bi.lsp || bi.len)); // quit after last stack level processed
    return rc;
}

/**
 * match string value of lists: a and b (including sublists, but not sibling lists)
 *
 * b may contain strings with trailing '*' which will
 * match any tail of the corresponding string in a
 *
 * @param a
 * @param b (may contain '*')
 * @return result of ASCII comparison: <0, 0, >0
 */
int match (elem_t *a, elem_t *b)
{
    int rc = 0;
    iter_t ai = { 0 };
    iter_t bi = { 0 };

    inititer_no_siblings(&ai, a);  // compare a and a's progeny
    inititer_no_siblings(&bi, b);  //    with b and b's progeny
    do {
        while (ai.len && bi.len && rc == 0) {
            if (*bi.cp == '*') { 
                //  "x..." matches "*"  where ai.len >= bi.len
                rc = 0;
                break;
            } 
            ai.len--;
            bi.len--;
            rc = (*ai.cp++) - (*bi.cp++);
        }
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
    } while (rc == 0 && (ai.lsp || ai.len) && (bi.lsp || bi.len)); // quit after last stack level processed
    return rc;
}


/**
 * printg - print the canonical g string representation of a list
 *
 * @param THREAD context
 * @param a - list to be printed
 */
static void printg (THREAD_t *THREAD, elem_t *a)
{
    iter_t ai = { 0 };
    writer_fn_t writer_fn = THREAD->writer_fn;

    inititer(&ai, a);
    do {
        writer_fn(THREAD, ai.cp, ai.len);
        nextiter(&ai);
    } while (ai.len || ai.lsp);
    writer_fn(THREAD, (unsigned char*)"\n", 1);
}

/**
 * print a tree from left to right.  i.e in insertion sort order
 *
 * @param THREAD context
 * @param p the root of the tree
 */
void printt(THREAD_t * THREAD, elem_t * p)
{
    if (p->u.t.left) {
        printt(THREAD, p->u.t.left);
    }
    printg(THREAD, p->u.t.key);
    if (p->u.t.right) {
        printt(THREAD, p->u.t.right);
    }
}
