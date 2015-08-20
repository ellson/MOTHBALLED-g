#include <stdio.h>
#include <assert.h>

#include "grammar.h"
#include "inbuf.h"
#include "list.h"
#include "context.h"
#include "emit.h"

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
pattern_r(container_context_t * CC, elem_t * subject, elem_t * pattern)
{
	elem_t *s_elem, *p_elem;
	unsigned char *s_cp, *p_cp;
	int s_len, p_len;

	s_elem = subject->u.list.first;
	p_elem = pattern->u.list.first;
	while (s_elem && p_elem) {
		if (s_elem->type != p_elem->type) {
			break;  // no match if one has reached FRAGELEMs without the other
		}
		if ((elemtype_t) (s_elem->type) == LISTELEM) {
			if (s_elem->state != p_elem->state) {
				break; // no match if the state structure is different,  e.g. OBJECT vs. OBJECT_LIST
			}
		    return (pattern_r(CC, s_elem, p_elem));   // recurse
		} else {	// FRAGELEM
			s_len = 0;
			p_len = 0;
			while (1)	// the fragmentation is not necessarily the same
			{
				if (s_len == 0) { // if we reached the end of a subject frag, try the next frag
					s_cp = s_elem->u.frag.frag;
					s_len = s_elem->v.frag.len;
					s_elem = s_elem->next;
				}
				if (p_len == 0) { // if we reached the end of a pattern frag, try the next frag
					p_cp = p_elem->u.frag.frag;
					p_len = p_elem->v.frag.len;
					p_elem = p_elem->next;
				}
				if (s_len == 0 && p_len == 0) {	// reached the end of both strings with no mismatch
					return SUCCESS; // so its a match
				}
				if (*p_cp == '*') {	// reached an '*' in the pattern
                                    //    - prefix match completed
                                    //    FIXME - no support here for suffix matching
					return SUCCESS; // so its a match 
				}
				if (s_len == 0 || p_len == 0) { // one reached the end without the other, so no match
					break;
				}
				if (*s_cp++ != *p_cp++) {  // test is chars match, if they do then move on to test the next chars
					break;  // else, no match
				}
			}
		}
	}
    return FAIL;
}

// Look for pattern match(es) to the current subject (segregated into NODE and EDGE patterns).
// For each match, insert a (refcounted copy) of the current subject, followed by (refcounted) copies
// of the ATTRIBUTES and CONTAINER from the pattern.  Finally insert the current subject again with 
// its own ATTRIBUTES and CONTENTS.
void pattern(container_context_t * CC, elem_t * subject)
{
	elem_t *pattern_acts, *nextpattern_act, *elem;

	if (CC->act_type == NODE) {
		pattern_acts = &(CC->node_pattern_acts);
	} else {
		assert(CC->act_type == EDGE);
		pattern_acts = &(CC->edge_pattern_acts);
	}
	nextpattern_act = pattern_acts->u.list.first;
	while (nextpattern_act) {

		elem = nextpattern_act->u.list.first;
		assert(elem);
		assert((state_t) elem->state == ACT);

		elem = elem->u.list.first;
		assert(elem);
		assert((state_t) elem->state == SUBJECT);

		if ((pattern_r(CC, subject->u.list.first, elem->u.list.first)) == SUCCESS) {
// FIXME - do insertion here  -- while we know what pattern was matched
fprintf(stdout,"..matched..");
        }

		nextpattern_act = nextpattern_act->next;
	}
}
