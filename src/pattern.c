/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <sys/types.h>
#include <assert.h>

#include "pattern.h"

static success_t
je_pattern_r(CONTENT_t * CONTENT, elem_t * subject, elem_t * pattern);

/*
 * A pattern is a SUBJECT in which one or more STRINGs contain an AST ('*')
 * The AST is a wild-card that matches any substring of zero or more 
 * characters at that position.
 *
 * An indivual STRING may have no more that one AST, but multiple
 * STRING in the SUBJECT may have AST
 *
 * The following are valid pattern STRING:
 *      *       // This and the next require only prefix matching
 *      abcd*
 *
 *      ab*ef   // FIXME - this and the next require suffix matching - not yet implemented
 *      *cdef
 *
 * and all of these patterns will match
 *      abcdef
 * 
 * A SUBJECT is matched if all its pattern and non-pattern STRINGS
 * match, after pattern substitution.
 * 
 * ENDPOINTSETs are not expanded in patterns, or in SUBJECTs
 * before pattern matching. (i.e. the form of ENDPOINTSETS must be the same for a match to occur)
 */ 
 
/**
 * Look for pattern match(es) to the current subject (segregated
 * into NODE and EDGE patterns).
 * For each match, append a (refcounted copy) of the current
 * subject, followed by (refcounted) copies of the ATTRIBUTES
 * and CONTAINER from the pattern.  Finally return for the current
 * subject to be appended with its own ATTRIBUTES and ACTIVITY.
 *
 * @param CONTENT container_context
 * @param root of the output tree
 * @param subject to be checked for pattern matches
 */
void je_pattern(CONTENT_t * CONTENT, elem_t * root, elem_t * subject)
{
    PARSE_t *C = CONTENT->C;
    LIST_t * LIST = (LIST_t *)C;
    elem_t *pattern_acts, *pact, *psubj, *pattr;

    assert(root);
    assert(subject);
    assert((state_t) subject->state == SUBJECT);

    if (CONTENT->subject_type == NODE) {
        pattern_acts = &(CONTENT->node_pattern_acts);
    } else {
        assert(CONTENT->subject_type == EDGE);
        pattern_acts = &(CONTENT->edge_pattern_acts);
    }

    // iterate over available patterns
    for ( pact = pattern_acts->u.l.first; pact; pact = pact->next) {
        assert((state_t) pact->state == ACT);
        psubj = pact->u.l.first;
        assert(psubj);
        assert((state_t) psubj->state == SUBJECT);
        pattr = psubj->next;

        // FIXME - contents from pattern ??
        if ((je_pattern_r(CONTENT, subject->u.l.first, psubj->u.l.first)) == SUCCESS) {
            // insert matched attrubutes, contents,
            // and then the subject again
            
            append_list(root, ref_list(LIST, subject));
            emit_subject(CONTENT, subject);
            if (pattr && (state_t)pattr->state == ATTRIBUTES) {
                append_list(root, ref_list(LIST, pattr));
                emit_attributes(CONTENT, pattr);
            }

            // FIXME -- contents

            C->stat_patternmatches++;
        }
    }
}

#if 1

// FIXME - this code is wrong - needs fixes from compare.c (attempted in #else)
// FIXME - better maybe, merge with compare.c and call from here

/**
 * attempt to match one pattern to a subject
 *
 * @param CONTENT container_context
 * @param subject 
 * @param pattern 
 * @return success/fail 
 */
static success_t
je_pattern_r(CONTENT_t * CONTENT, elem_t * subject, elem_t * pattern)
{
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
        if ((elemtype_t) (s_elem->type) == LISTELEM) {
            if (ts_elem->state != tp_elem->state) {
                return FAIL;    // no match if the state structure is different,  e.g. OBJECT vs. OBJECT_LIST
            }
            while (ts_elem || tp_elem) {    // quick test before recursing...
                if (!(ts_elem && tp_elem)) {
                    return FAIL;    // no match if the number of elems ism't the same
                }
                ts_elem = ts_elem->next;
                tp_elem = tp_elem->next;
            }
            if ((je_pattern_r(CONTENT, s_elem, p_elem)) == FAIL) {  // recurse
                return FAIL;
            }
        } else {    // FRAGELEM
            while (ts_elem && tp_elem) {
                // the fragmentation is not necessarily
                // the same so manage ts_elem and tp_elem
                // separately
                if (s_len == 0) {    // if we reached the end
                        // of a subject frag, try the next frag
                    s_cp = ts_elem->u.f.frag;
                    s_len = ts_elem->len;
                    ts_elem = ts_elem->next;
                }
                if (p_len == 0) {    // if we reached the end
                        // of a pattern frag, try the next frag
                    p_cp = tp_elem->u.f.frag;
                    p_len = tp_elem->len;
                    tp_elem = tp_elem->next;
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
        }
        if (p_len) {  //must match the entire pattern
                return FAIL;
        }
        s_elem = s_elem->next;
        p_elem = p_elem->next;
    }
    return SUCCESS;
}
#else
/**
 * attempt to match one pattern to a subject
 *
 * @param CONTENT container_context
 * @param subject 
 * @param pattern 
 * @return success/fail 
 */

static success_t
je_pattern_r(CONTENT_t * CONTENT, elem_t * subject, elem_t * pattern)
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
                return FAIL;    // no match if the state structure is different,  e.g. OBJECT vs. OBJECT_LIST
            }
            while (ts_elem || tp_elem) {    // quick test before recursing...
                if (!(ts_elem && tp_elem)) {
                    return FAIL;    // no match if the number of elems ism't the same
                }
                ts_elem = ts_elem->next;
                tp_elem = tp_elem->next;
            }
            if ((je_pattern_r(CONTENT, s_elem, p_elem)) == FAIL) {  // recurse
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
                    if ((ts_elem = ts_elem->next)) { // try the next frag
                        s_cp = ts_elem->u.f.frag;
                        s_len = ts_elem->len;
                    }
                }
                if (p_len == 0) {    // if we reached the end of "b" frag
                    if ((tp_elem = tp_elem->next)) { // try the next frag
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
        s_elem = s_elem->next;
        p_elem = p_elem->next;
    }
    return SUCCESS;
}
#endif
