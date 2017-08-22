/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "thread.h"
#include "iter_print.h"

static size_t stdout_writer(const void *ptr, size_t size)
{
    return fwrite(ptr, size, 1, stdout);
}

/**
 * printg - print the canonical g string representation of a list
 *
 * @param THREAD context
 * @param a - list to be printed
 */
void printg (THREAD_t *THREAD, elem_t *a)
{
    iter_t ai = { 0 };

    inititer(&ai, a, &stdout_writer);
    do {
        ai.writer_fn(ai.cp, ai.len);
        nextiter(&ai);
    } while (ai.len || ai.lsp);
    ai.writer_fn("\n", 1);
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

//  ------------------------ ikea bits ----------------
//

static void ikea_flush(THREAD_t *THREAD)
{
    ikea_box_append(THREAD->ikea_box, THREAD->buf, THREAD->pos);
    THREAD->pos = 0;
}

static void ikea_putc(THREAD_t *THREAD, char c)
{
    THREAD->buf[THREAD->pos++] = c;
    if (THREAD->pos >= sizeof(THREAD->buf)) {
        ikea_flush(THREAD);
    }
}

/**
 * ikea_printg - print the canonical g string representation of a list
 *
 * @param THREAD context
 * @param a - list to be printed
 */
static void ikea_printg (THREAD_t *THREAD, elem_t *a)
{
    iter_t ai = { 0 };

    inititer(&ai, a, &stdout_writer);
    do {
#if 1
        while (ai.len) {
            unsigned char c = *ai.cp++;
            ai.len--;
            ikea_putc(THREAD, c);
        }
#else
        ai.writer_fn("\n", 1);
#endif
        nextiter(&ai);
    } while (ai.len || ai.lsp);
#if 1
    ikea_putc(THREAD, '\n');   // canonical form,  '\n' between ACTs
#else
    ai.writer_fn("\n", 1);
#endif
}

/**
 * ikea_printt - print a tree from left to right. i.e in insertion sort order
 *
 * @param THREAD context
 * @param p the root of the tree
 */
void ikea_printt(THREAD_t * THREAD, elem_t * p)
{
    if (p->u.t.left) {
        ikea_printt(THREAD, p->u.t.left);
    }
    ikea_printg(THREAD, p->u.t.key);
    if (p->u.t.right) {
        ikea_printt(THREAD, p->u.t.right);
    }
    ikea_flush(THREAD);
}

