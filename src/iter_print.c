/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "thread.h"
#include "iter_print.h"

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
    writer_fn(THREAD, "\n", 1);
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
