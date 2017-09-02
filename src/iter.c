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
#include <string.h>
#include <assert.h>

#include "types.h"
#include "inbuf.h"
#include "list.h"
#include "grammar.h"
#include "thread.h"
#include "iter.h"

// separator strings:       minimal   pretty
char *sep_begtree[4]    = { "","",    "",""     };
char *sep_intree[4]     = { " ","",   "\n  ","" };
char *sep_skip[4]       = { " ","",   " ",""    };
char *sep_step[4]       = { "","",    "",""     };
char *sep_ACT[4]        = { "","",    "",""     };
char *sep_ATTRIBUTES[4] = { "[","]",  " [\n  ","\n]" };
char *sep_DISAMBIG[4]   = { "`","",   "`",""    };
char *sep_EDGE[4]       = { "<",">",  "<",">"   };
char *sep_KID[4]        = { "/","",   "/",""    };
char *sep_MUM[4]        = { "^","",   "^",""    };
char *sep_PORT[4]       = { ":","",   ":",""    };
char *sep_SET[4]        = { "(",")",  "(",")"   };
char *sep_SIS[4]        = { " ","",   " ",""    };
char *sep_VALUE[4]      = { "=","",   " = ",""  };

static void sep(iter_t *iter, int idx)
{
    char *cp = iter->lstack[iter->lsp].sep[idx + iter->pretty];
    iter->len = strlen(cp);
    iter->cp = (unsigned char*)cp;
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
        iter->lstack[iter->lsp].lnx = this->u.f.next;
        iter->cp = this->u.f.frag;
        iter->len = this->len;
        break;
    case SHORTSTRELEM:
        iter->lstack[iter->lsp].lnx = NULL;
        iter->cp = this->u.s.str;
        iter->len = this->len;
        break;
    case LISTELEM:
        if (this->u.l.first && (elemtype_t)this->u.l.first->type == FRAGELEM) {
            // align with SHORTSTRELEM
            this = this->u.l.first;
            iter->lstack[iter->lsp].lnx = this->u.f.next;
            iter->cp = this->u.f.frag;
            iter->len = this->len;
        } else {
            assert(iter->lsp < MAXNEST);
            switch ((state_t)this->state) {
                case ACT:
                    iter->lstack[iter->lsp].sep = sep_ACT;
                    break;
                case EDGE:
                    iter->lstack[iter->lsp].sep = sep_EDGE;
                    break;
                case MUM:
                    iter->lstack[iter->lsp].sep = sep_MUM;
                    break;
                case SET:
                case ENDPOINTSET:
                    iter->lstack[iter->lsp].sep = sep_SET;
                    break;
                case ATTRID:
                    // FIXME - This is a hack! Probably the whole
                    //    spacing character scheme needs to be rethunk.
                    if (iter->intree) {
                        iter->lstack[iter->lsp].sep = sep_intree;
                    }
                    else {
                        // suppress extra space before the attr=value list..
                        iter->lstack[iter->lsp].sep = sep_begtree;
                    }
                    break;
                default:
                    iter->lstack[iter->lsp].sep = sep_step;
                    break;
            }
            sep(iter,0);
            iter->lstack[iter->lsp++].lnx = this->u.l.next;
            iter->lstack[iter->lsp].lnx = this->u.l.first;
        }
        break;
    case TREEELEM:
        if (iter->tsp == 0) {
            iter->tsp++;
            iter->tstack[iter->tsp].tnx = this;
            iter->tstack[iter->tsp].dir = 0;
            iter->len = 0;    // this suppresses a second '[' 
            iter->intree = 0; // used to suppress extra space before the attr=value list..
        } else {
            iter->intree = 1;
        }
        do {
            if  (iter->tstack[iter->tsp].dir == 0) {
                iter->tstack[iter->tsp].dir++;
                next = iter->tstack[iter->tsp].tnx;
                if (next->u.t.left) {
                    iter->tsp++;
                    assert (iter->tsp < MAXNEST);
                    iter->tstack[iter->tsp].tnx = next->u.t.left;
                    iter->tstack[iter->tsp].dir = 0;
                    continue;
                }
            }
            if  (iter->tstack[iter->tsp].dir == 1) {
                next = iter->tstack[iter->tsp].tnx;
                iter->tstack[iter->tsp].dir++;
                first = next->u.t.key;
                break;
            }
            if (iter->tstack[iter->tsp].dir == 2) {
                iter->tstack[iter->tsp].dir++;
                next = iter->tstack[iter->tsp].tnx;
                if (next->u.t.right) {
                    iter->tsp++;
                    assert (iter->tsp < MAXNEST);
                    iter->tstack[iter->tsp].tnx = next->u.t.right;
                    iter->tstack[iter->tsp].dir = 0;
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
        iter->lstack[iter->lsp++].lnx = next;
        iter->lstack[iter->lsp].lnx = first;
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
        this = iter->lstack[--iter->lsp].lnx;
        if (this) {
            switch ((elemtype_t)this->type) {
                case TREEELEM:
                    iter->len = 0;
                    break;
                default:
                    switch ((state_t)this->state) {
                        // elems that follow elems of a different state_t
                        // (non-homogenous lists) need to over-ride the
                        // pop_space_push of the preceding elem
                        case ATTRIBUTES:
                            iter->lstack[iter->lsp].sep = sep_ATTRIBUTES;
                            break;
                        case DISAMBIG:
                            iter->lstack[iter->lsp].sep = sep_DISAMBIG;
                            break;
                        case VALUE:
                            iter->lstack[iter->lsp].sep = sep_VALUE;
                            break;
//                      case SIS:
//                          iter->lstack[iter->lsp].sep = sep_SIS;
//                          break;
                        case KID:
                            iter->lstack[iter->lsp].sep = sep_KID;
                            break;
                        case PORT:
                            iter->lstack[iter->lsp].sep = sep_PORT;
                            break;
                        default:
                            iter->lstack[iter->lsp].sep = sep_skip;
                            break;
                    }
                    sep(iter,0);
                    iter->lstack[iter->lsp++].lnx = this->u.l.next;
                    iter->lstack[iter->lsp].lnx = this->u.l.first;
                    break;
            }
        } else {
            sep(iter,1);
        }
    } else {
        iter->len = 0;
    }
}

/**
 * initialize an iterator for traversing elem, its progeny, and its siblings
 *
 * @param iter - a struct containg the current state of the iterator
 * @param elem - the root elem of the list to be iterated
 * @param pretty - 0 minimum (canonical) spacing, 1 to print attributes on their own line
 */
static void inititer(iter_t *iter, elem_t *elem, int pretty)
{
    assert(iter);
    assert(elem);
    assert((elemtype_t)elem->type == LISTELEM
        || (elemtype_t)elem->type == SHORTSTRELEM);
    iter->lsp = 0;
    iter->pretty = pretty?2:0; // iter->pretty must be 0 or 2
    stepiter(iter, elem);
}

/**
 * initialize an iterator for traversing elem and its progeny only (no siblings)
 *
 * @param iter - a struct containing the current state of the iterator
 * @param elem - the root elem of the list to be iterated
 * @param pretty - 0 minimum (canonical) spacing, 1 to print attributes on their own line
 */
static void inititer_no_siblings(iter_t *iter, elem_t *elem, int pretty)
{
    assert(iter);
    assert(elem);
    assert((elemtype_t)elem->type == LISTELEM
        || (elemtype_t)elem->type == SHORTSTRELEM);
    iter->lsp = 0;
    iter->pretty = pretty?2:0; // iter->pretty must be 0 or 2
    stepiter(iter, elem);
    iter->lstack[0].lnx = NULL;
}

/**
 * move the iterator to the next elem in traversal order
 *
 * @param iter - a struct containing the current state of the iterator
 */
static void nextiter(iter_t *iter)
{
    elem_t *this = iter->lstack[iter->lsp].lnx;

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

    inititer_no_siblings(&ai, a, 0);  // compare a and a's progeny
    inititer_no_siblings(&bi, b, 0);  //    with b and b's progeny
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

    inititer_no_siblings(&ai, a, 0);  // compare a and a's progeny
    inititer_no_siblings(&bi, b, 0);  //    with b and b's progeny
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
            if (ai.len && *bi.cp == '*') {
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


// FIXME - printg needs option to print _contenhash (for ikea strorage)
//           or to expand it (canonical output)

// FIXME - need an option (or separate filter) to print in .gv format.
//            will need edges with other than 2-legs to be expanded
//            with a point-node.

// FIXME - maybe additional filtering for .gv

/**
 * printg - print the canonical g string representation of a list
 *
 * @param THREAD context
 * @param a - list to be printed
 */
static void printg (THREAD_t *THREAD, elem_t *a)
{
    iter_t ai = { 0 };
    out_write_fn_t out_write_fn = THREAD->out_disc->out_write_fn;

    inititer(&ai, a, THREAD->PROCESS->flags & 2);
    do {
        out_write_fn(THREAD, ai.cp, ai.len);
        nextiter(&ai);
    } while (ai.len || ai.lsp);
    out_write_fn(THREAD, (unsigned char*)"\n", 1);
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
