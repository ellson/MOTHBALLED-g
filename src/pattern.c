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
//      *
//      abcd*
//      ab*ef   // FIXME - this and the next require tail matching of the STRING - not yet implemented
//      *cdef
//
// and all of these patterns will match
//      abcdef

// A SUBJECT is matched if all its pattern and non-pattern STRINGS
// match, after pattern substitution.

// ENDPOINTSETs are not expanded in patterns, or in SUBJECTss
// before pattern matching.

static void
pattern_r(container_context_t * CC, elem_t * subject, elem_t * pattern)
{
	elem_t *s_elem, *p_elem;
	unsigned char *s_cp, *p_cp;
	int s_len, p_len;

	s_elem = subject->u.list.first;
	p_elem = pattern->u.list.first;
	while (s_elem && p_elem) {
		if (s_elem->type != p_elem->type) {
			break;
		}
		if ((elemtype_t) (s_elem->type) == LISTELEM) {
			if (s_elem->state != p_elem->state) {
				break;
			}
		} else		// FRAGELEM
		{
			s_len = 0;
			p_len = 0;
			while (1)	// the fragmentation is not necessarily the same
			{
				if (s_len == 0) {
					s_cp = s_elem->u.frag.frag;
					s_len = s_elem->v.frag.len;
					s_elem = s_elem->next;
				}
				if (p_len == 0) {
					p_cp = p_elem->u.frag.frag;
					p_len = p_elem->v.frag.len;
					p_elem = p_elem->next;
				}
				if (s_len == 0 && p_len == 0) {	// match
					fprintf(stdout, "\npattern match\n");
					return;
					break;
				}
				if (*p_cp == '*') {	// match
					fprintf(stdout,
						"\npattern match trailing *\n");
					return;
					break;
				}
				if (s_len == 0 || p_len == 0) {
					break;
				}
				if (*s_cp++ != *p_cp++) {
					break;
				}
			}
		}
	}
	fprintf(stdout, "\nno match\n");
}

// Look for pattern match(es) to the current subject (segresgtated into NODE and EDGE patterns).
// For each match, insert a (refcounted copy) of the current subject, followed by (refcounted) copies
// of the ATTRIBUTES and CONTAINER from the pattern.  Finally insert the current subject again with 
// its own ATTRIBUTES and CONTENTS.
success_t pattern(container_context_t * CC, elem_t * subject)
{
	success_t rc;
	elem_t *pattern_acts, *nextpattern_act, *elem;

#define DBUG
#ifdef DBUG
	putc('\n', stdout);
	putc('s', stdout);
#endif
	if (CC->act_type == NODE) {
#ifdef DBUG
		putc('n', stdout);
#endif
		pattern_acts = &(CC->node_pattern_acts);
	} else {
		assert(CC->act_type == EDGE);
#ifdef DBUG
		putc('e', stdout);
#endif
		pattern_acts = &(CC->edge_pattern_acts);
	}
#ifdef DBUG
	putc(' ', stdout);
	print_list(stdout, subject, 3, ' ');
	putc('\n', stdout);
#endif
	nextpattern_act = pattern_acts->u.list.first;
	while (nextpattern_act) {

		elem = nextpattern_act->u.list.first;
		assert(elem && (state_t) elem->state == ACT);

#ifdef DBUG
		putc('\n', stdout);
		putc('p', stdout);
		putc(' ', stdout);
		putc(' ', stdout);
		print_list(stdout, elem->u.list.first, 3, ' ');
		putc('\n', stdout);
#endif

		pattern_r(CC, subject->u.list.first, elem->u.list.first);

		nextpattern_act = nextpattern_act->next;
	}

	rc = SUCCESS;
	return rc;
}
