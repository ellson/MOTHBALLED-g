/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "inbuf.h"
#include "grammar.h"
#include "list.h"
#include "compare.h"
#include "frag.h"

/**
 * compare string value of a and b - recursive function
 *
 * @param a 
 * @param b 
 * @return <0, 0 >0
 */
int je_compare(elem_t * a, elem_t * b)
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
        a_len = 0;
        b_len = 0;
        if ((elemtype_t) (a_elem->type) == LISTELEM) {
            return je_compare(a_elem, b_elem);
        } else {    // FRAGELEM
            while (ta_elem && tb_elem) {
                // the fragmentation is not necessarily
                // the same so manage ta_elem and tb_elem
                // separately
                if (a_len == 0) {    // if we reached the end
                        // of an "a" frag, try the next frag
                    a_cp = ta_elem->u.f.frag;
                    a_len = ta_elem->len;
                    ta_elem = ta_elem->next;
                }
                if (b_len == 0) {    // if we reached the end
                        // of a "b" frag, try the next frag
                    b_cp = tb_elem->u.f.frag;
                    b_len = tb_elem->len;
                    tb_elem = tb_elem->next;
                }
                a_len--;
                b_len--;
                if (*a_cp++ != *b_cp++) {    // test if chars match
                                             // (and optimistically increment)
                    return *(--a_cp) - *(--b_cp);
                }
            }
            if (a_len != b_len) {  //must match the entire string
                return a_len - b_len;
            }
            // all matched so far, move on to test the next STRING
        }
        a_elem = a_elem->next;
        b_elem = b_elem->next;
    }
    return 0;
}

elem_t * je_merge(elem_t * new, elem_t * old)
{
    old->refs--;
    new->refs++;
    return new;
}
