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
 * compare string value of lists: a and b (not extending to siblings)
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

    inititer0(&ai, a);  // compare a and a's progeny
    inititer0(&bi, b);  //    with b and b's progeny
                        // i.e. do not extend to siblings of a or b
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
 * match string value of lists: a and b (not extending to siblings)
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
    int rc;
    iter_t ai = { 0 };
    iter_t bi = { 0 };

    inititer0(&ai, a);  // compare a and a's progeny
    inititer0(&bi, b);  //    with b and b's progeny
                        // i.e. do not extend to siblings of a or b
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
