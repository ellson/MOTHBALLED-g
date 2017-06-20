/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "thread.h"
#include "iter_print.h"

static void bufflush(THREAD_t *THREAD)
{
    fwrite(THREAD->buf, THREAD->pos, 1, TOKEN()->out);
    // FIXME - error handling
    THREAD->pos = 0;
}

static void bufputc(THREAD_t *THREAD, char c)
{
    THREAD->buf[THREAD->pos++] = c;
    if (THREAD->pos >= sizeof(THREAD->buf)) {
        bufflush(THREAD);
    }
}

size_t ikea_stdout_writer(const void *ptr, size_t size)
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
    char c;

    inititer(&ai, a, &ikea_stdout_writer);
    do {
        while (ai.len) {
            ai.len--;
            c = *ai.cp++;
            if (c) {
                bufputc(THREAD, c);
            }
        }
        nextiter(&ai);
    } while (ai.len || ai.lsp);
    bufputc(THREAD, '\n');   // canonical form,  '\n' between ACTs
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
    bufflush(THREAD);
}

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
    char c;

    inititer(&ai, a, &ikea_stdout_writer);
    do {
        while (ai.len) {
            ai.len--;
            c = *ai.cp++;
            if (c) {
                ikea_putc(THREAD, c);
            }
        }
        nextiter(&ai);
    } while (ai.len || ai.lsp);
    ikea_putc(THREAD, '\n');   // canonical form,  '\n' between ACTs
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

