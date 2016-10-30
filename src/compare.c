/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "compare.h"

/**
 * compare string value of a and b - recursive function
 *
 * @param a 
 * @param b 
 * @return <0, 0 >0
 */
int compare(elem_t * a, elem_t * b)
{
    elem_t *a_elem, *b_elem, *ta_elem, *tb_elem;
    unsigned char *a_cp, *b_cp;
    uint16_t a_len, b_len;

    a_elem = a->u.l.first;
    b_elem = b->u.l.first;
    while (a_elem && b_elem) {
        assert(a_elem->type == b_elem->type);  // list structure must match
        ta_elem = a_elem;
        tb_elem = b_elem;
        if ((elemtype_t) (a_elem->type) == LISTELEM) {
            return compare(a_elem, b_elem);
        } else {    // FRAGELEM
            a_cp = ta_elem->u.f.frag;
            a_len = ta_elem->len;
            b_cp = tb_elem->u.f.frag;
            b_len = tb_elem->len;
            while (1) {
                // the fragmentation is not necessarily
                // the same so manage ta_elem and tb_elem
                // separately
                if (a_len == 0) {    // if we reached the end of "a" frag
                    if ((ta_elem = ta_elem->u.f.next)) { // try the next frag
                        a_cp = ta_elem->u.f.frag;
                        a_len = ta_elem->len;
                    }
                }
                if (b_len == 0) {    // if we reached the end of "b" frag
                    if ((tb_elem = tb_elem->u.f.next)) { // try the next frag
                        b_cp = tb_elem->u.f.frag;
                        b_len = tb_elem->len;
                    }
                }
                if (! (a_len && b_len)) { // at least one has reached the end
                    break;
                }
                if (*a_cp != *b_cp) {    // test if chars match
                    return *a_cp - *b_cp;
                }
                a_cp++;                           
                b_cp++;                           
                a_len--;
                b_len--;
            }
            if (a_len || b_len) {  // if strings are same length then
                                   // both should be 0, we know that one is 0
                return a_len - b_len;   // longer strings sort later
            }
            // all matched so far, move on to test the next STRING
        }
        a_elem = a_elem->u.l.next;
        b_elem = b_elem->u.l.next;
    }
    return 0;   // if we get here, then the strings match
}
