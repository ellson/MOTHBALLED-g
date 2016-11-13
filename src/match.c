/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <sys/types.h>
#include <assert.h>

#include "thread.h"
#include "match.h"


// FIXME - this code is wrong - needs fixes from compare.c (attempted in #else)

#if 1

/**
 * attempt to match one pattern to a subject
 *
 * @param CONTAINER containers context
 * @param subject 
 * @param pattern 
 * @return success/fail 
 */
success_t match(CONTAINER_t * CONTAINER, elem_t * subject, elem_t * pattern)
{
    THREAD_t *THREAD = CONTAINER->THREAD;
    elem_t *s_elem, *p_elem, *ts_elem, *tp_elem;
    unsigned char *s_cp, *p_cp;
    int s_len, p_len;

    s_elem = subject->u.l.first;
    p_elem = pattern->u.l.first;
    while (s_elem && p_elem) {
        if (s_elem->type != p_elem->type) {
            return FAIL;    // no match if one has reached FRAGELEMs without the other
        }
        ts_elem = s_elem;
        tp_elem = p_elem;
        s_len = 0;
        p_len = 0;
        if (ts_elem->state != tp_elem->state) {
            return FAIL;    // no match if the state structure is different
        }
        switch ((elemtype_t)s_elem->type) {
        case LISTELEM:
            if (ts_elem->len != tp_elem->len) {
                return FAIL;    // no match if the number of elems ism't the same
            }
            if ((match(CONTAINER, s_elem, p_elem)) == FAIL) {  // recurse
                return FAIL;
            }
            break;
        case FRAGELEM:
            while (ts_elem && tp_elem) {
                // the fragmentation is not necessarily
                // the same so manage ts_elem and tp_elem
                // separately
                if (s_len == 0) {    // if we reached the end
                        // of a subject frag, try the next frag
                    s_cp = ts_elem->u.f.frag;
                    s_len = ts_elem->len;
                    ts_elem = ts_elem->u.f.next;
                }
                if (p_len == 0) {    // if we reached the end
                        // of a pattern frag, try the next frag
                    p_cp = tp_elem->u.f.frag;
                    p_len = tp_elem->len;
                    tp_elem = tp_elem->u.f.next;
                }
                s_len--;
                p_len--;
                if (*p_cp == '*') {    // reached an '*' in the pattern
                    //    - prefix match completed
                    //    FIXME - no support here for suffix matching
                    break;
                }
                if (*s_cp++ != *p_cp++) {    // test if chars match
                    return FAIL;    // else, no match
                }
            }
            // all matched so far, move on to test the next STRING
            break;
        default:
            assert(0);
            break;
        }
        if (p_len) {  //must match the entire pattern
                return FAIL;
        }
        s_elem = s_elem->u.l.next;
        p_elem = p_elem->u.l.next;
    }
    return SUCCESS;
}
#else
/**
 * attempt to match one pattern to a subject
 *
 * @param CONTAINER container_context
 * @param subject 
 * @param pattern 
 * @return success/fail 
 */

static success_t
pattern_r(CONTAINER_t * CONTAINER, elem_t * subject, elem_t * pattern)
{
    elem_t *s_elem, *p_elem, *ts_elem, *tp_elem;
    unsigned char *s_cp, *p_cp;
    uint16_t s_len, p_len;

    s_elem = subject->u.l.first;
    p_elem = pattern->u.l.first;
    while (s_elem && p_elem) {
        if (s_elem->type != p_elem->type) {
            return FAIL;    // no match if one has reached FRAGELEMs without the other
        }
        ts_elem = s_elem;
        tp_elem = p_elem;
        if ((elemtype_t) (s_elem->type) == LISTELEM) {
            if (ts_elem->state != tp_elem->state) {
                return FAIL;    // no match if the state structure is different
            }
            while (ts_elem || tp_elem) {    // quick test before recursing...
                if (!(ts_elem && tp_elem)) {
                    return FAIL;    // no match if the number of elems ism't the same
                }
                ts_elem = ts_elem->u.l.next;
                tp_elem = tp_elem->u.l.next;
            }
            if ((pattern_r(CONTAINER, s_elem, p_elem)) == FAIL) {  // recurse
                return FAIL;
            }
        } else {    // FRAGELEM
            s_cp = ts_elem->u.f.frag;
            s_len = ts_elem->len;
            p_cp = tp_elem->u.f.frag;
            p_len = tp_elem->len;
            while (1) {
                // the fragmentation is not necessarily
                // the same so manage ts_elem and tp_elem
                // separately
                if (s_len == 0) {    // if we reached the end of "a" frag
                    if ((ts_elem = ts_elem->u.f.next)) { // try the next frag
                        s_cp = ts_elem->u.f.frag;
                        s_len = ts_elem->len;
                    }
                }
                if (p_len == 0) {    // if we reached the end of "b" frag
                    if ((tp_elem = tp_elem->u.f.next)) { // try the next frag
                        p_cp = tp_elem->u.f.frag;
                        p_len = tp_elem->len;
                    }
                }
                if (! (s_len && p_len)) { // at least one has reached the end
                    break;
                }
                if (*p_cp == '*') {    // reached an '*' in the pattern
                    //    - prefix match completed
                    //    FIXME - no support here for suffix matching
                    break;
                }
                if (*s_cp != *p_cp) {    // test if chars match
                    return FAIL;
                }
                s_cp++;
                p_cp++;
                s_len--;
                p_len--;
            }
            // all matched so far, move on to test the next STRING
        }
        if (p_len) {  //must match the entire pattern
            return FAIL;
        }
        s_elem = s_elem->u.l.next;
        p_elem = p_elem->u.l.next;
    }
    return SUCCESS;
}
#endif
