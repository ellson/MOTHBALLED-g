/* vim:set shiftwidth=4 ts=8 expandtab: */

#include "libje_private.h"

// A pattern is a SUBJECT in which one or more STRINGs contain an AST ('*')
// The AST is a wild-card that matches any substring of zero or more 
// characters at that position.
//
// An indivual STRING may have no more that one AST, but multiple
// STRING in the SUBJECT may have AST
//
// The following are valid pattern STRING:
//      *       // This and the next require only prefix matching
//      abcd*
//
//      ab*ef   // FIXME - this and the next require suffix matching - not yet implemented
//      *cdef
//
// and all of these patterns will match
//      abcdef

// A SUBJECT is matched if all its pattern and non-pattern STRINGS
// match, after pattern substitution.

// ENDPOINTSETs are not expanded in patterns, or in SUBJECTss
// before pattern matching.

static success_t
je_pattern_r(container_context_t * CC, elem_t * subject, elem_t * pattern)
{
	elem_t *s_elem, *p_elem, *ts_elem, *tp_elem;
	unsigned char *s_cp, *p_cp;
	int s_len, p_len;

	s_elem = subject->u.list.first;
	p_elem = pattern->u.list.first;
	while (s_elem && p_elem) {
		if (s_elem->type != p_elem->type) {
			return FAIL;	// no match if one has reached FRAGELEMs without the other
		}
		ts_elem = s_elem;
		tp_elem = p_elem;
		if ((elemtype_t) (s_elem->type) == LISTELEM) {
			if (ts_elem->state != tp_elem->state) {
				return FAIL;	// no match if the state structure is different,  e.g. OBJECT vs. OBJECT_LIST
			}
			while (ts_elem || tp_elem) {	// quick test before recursing...
				if (!(ts_elem && tp_elem)) {
					return FAIL;	// no match if the number of elems ism't the same
				}
				ts_elem = ts_elem->next;
				tp_elem = tp_elem->next;
			}
			if ((je_pattern_r(CC, s_elem, p_elem)) == FAIL) {  // recurse
				return FAIL;
			}
		} else {	// FRAGELEM
			s_len = 0;
			p_len = 0;
			while (ts_elem && tp_elem) {
				// the fragmentation is not necessarily
				// the same so manage ts_elem and tp_elem
				// separately
				if (s_len == 0) {	// if we reached the end
						// of a subject frag, try the next frag
					s_cp = ts_elem->u.frag.frag;
					s_len = ts_elem->v.frag.len;
					ts_elem = ts_elem->next;
				}
				if (p_len == 0) {	// if we reached the end
						// of a pattern frag, try the next frag
					p_cp = tp_elem->u.frag.frag;
					p_len = tp_elem->v.frag.len;
					tp_elem = tp_elem->next;
				}
				s_len--;
				p_len--;
				if (*p_cp == '*') {	// reached an '*' in the pattern
					//    - prefix match completed
					//    FIXME - no support here for suffix matching
					break;	// so its a match 
				}
				if (*s_cp++ != *p_cp++) {	// test if chars match
					return FAIL;	// else, no match
				}
			}
			// all matched so far, move on to test the next STRING
		}
		s_elem = s_elem->next;
		p_elem = p_elem->next;
	}
	return SUCCESS;
}

// Look for pattern match(es) to the current subject (segregated
// into NODE and EDGE patterns).
// For each match, append a (refcounted copy) of the current
// subject, followed by (refcounted) copies of the ATTRIBUTES
// and CONTAINER from the pattern.  Finally return for the current
// subject to be appended with its own ATTRIBUTES and CONTENTS.
void je_pattern(container_context_t * CC, elem_t * root, elem_t * subject)
{
	elem_t *pattern_acts, *nextpattern_act, *elem, *psubj, *pattr;
    context_t *C = CC->context;

    assert(root);
    assert(subject);
    assert((state_t) subject->state == SUBJECT);

	if (CC->subject_type == NODE) {
		pattern_acts = &(CC->node_pattern_acts);
	} else {
		assert(CC->subject_type == EDGE);
		pattern_acts = &(CC->edge_pattern_acts);
	}
	nextpattern_act = pattern_acts->u.list.first;
	while (nextpattern_act) {

		elem = nextpattern_act->u.list.first;
        if (! elem) {
            break;
        }
		assert((state_t) elem->state == ACT);

		elem = elem->u.list.first;
        psubj = elem;
		assert(psubj);
		assert((state_t) psubj->state == SUBJECT);

		elem = elem->next;
		pattr = elem;

		// FIXME - contents from pattern ??

		if ((je_pattern_r(CC, subject->u.list.first, psubj->u.list.first)) == SUCCESS) {
			// insert matched attrubutes, contents,
			// and then the subject again
			
			elem = ref_list(C, subject);
			append_list(root, elem);
            emit_subject(CC, subject);

            if (pattr && (state_t)pattr->state == ATTRIBUTES) {
                elem = ref_list(C, pattr);
                append_list(root, elem);
                emit_attributes(CC, pattr);
            }

			// FIXME -- contents

            C->stat_patternmatches++;
		}

		nextpattern_act = nextpattern_act->next;
	}
}
