/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "types.h"
#include "inbuf.h"
#include "list.h"
#include "iter.h"
#include "compare.h"

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
        while (ai.len && bi.len && rc == 0) {  // itersep may be zero length
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
