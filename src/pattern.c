#include <stdio.h>
#include <assert.h>

#include "grammar.h"
#include "inbuf.h"
#include "list.h"
#include "context.h"
#include "emit.h"

// rewrite list into new list with any EQL elements substituted from oldlist
// A pattern is a SUBJECT in which one of more STRINGs contain an AST ('*')
// The AST is a wild-card that matches any substring of zero or more 
// characters at that position.
//
// An indivual STRING may have no more that one AST, but multiple
// STRING in the SUBJECT may have AST
//
// The following are valid pattern STRING:
// 	*
// 	ab*ef
//      *cdef
//      abcd*
//
// and all of these patterns will match
//      abcdef

// A SUBJECT is matched if all its pattern and non-pattern STRINGS
// match, after pattern substitution.

// ENDPOINTSETs are not expanded in patterns, or in SUBJECTss
// before pattern matching.

static void
pattern_r (container_context_t * CC, elem_t * subject, elem_t * pattern_subject)
{
#if 0
  elem_t *elem, *new, *nextoldelem = NULL;
  elem_t object = { 0 };
  state_t si;

  assert (list->type == (char) LISTELEM);

  elem = list->u.list.first;
  while (elem)
    {
      si = (state_t) elem->state;
      switch (si)
	{
	case NODE:
	case EDGE:
	  if (newlist->state == 0)
	    {
	      newlist->state = si;
	    }
	  else
	    {
	      if (si != (state_t) newlist->state)
		{
		  if (si == NODE)
		    {
		      emit_error (CC->context, si, "EDGE subject includes");
		    }
		  else
		    {
		      emit_error (CC->context, si, "NODE subject includes");
		    }
		}
	    }
	  if (*nextold)
	    {
              if (si == (state_t)(*nextold)->state)  // old subject matches NODE or EDGE type
                {
	          // doesn't matter if old is shorter
	          // ... as long as no forther substitutions are needed
	          nextoldelem = (*nextold)->u.list.first;	// in the recursion, iterate over the members of the NODE or EDGE SUBJECT
	          *nextold = (*nextold)->next;	// at this level, continue over the NODES or EDGES
                }
              else  // else we have no old, just ignore it
		{
		  nextoldelem = NULL;
                  *nextold = NULL;
                }
	    }
	  pattern_r (CC, elem, &nextoldelem, &object);	// recurse, adding result to a sublist
	  new = move_list (si, &object);
	  append_list (newlist, new);
	  break;
	case NODEID:
	  new = ref_list (elem->state, elem);
	  append_list (newlist, new);
	  if (*nextold)
	    {			// doesn't matter if old is shorter
	      // ... as long as no forther substitutions are needed
	      *nextold = (*nextold)->next;
	    }
	  break;
	case EQL:
	  if (*nextold)
	    {
	      new = ref_list ((*nextold)->state, *nextold);
	      append_list (newlist, new);

	      *nextold = (*nextold)->next;
	    }
	  else
	    {
	      emit_error (CC->context, si,
			  "No corresponding object found for same-as substitution");
	    }
	  break;
	default:
	  if (*nextold)
	    {			// doesn't matter if old is shorter
	      // ... as long as no forther substitutions are needed
	      nextoldelem = (*nextold)->u.list.first;	// in the recursion, iterate over the members of the NODE or EDGE SUBJECT
	      *nextold = (*nextold)->next;	// at this level, continue over the NODES or EDGES
	    }
	  pattern_r (CC, elem, &nextoldelem, &object);	// recurse, adding result to a sublist
	  new = move_list (si, &object);
	  append_list (newlist, new);
	  break;
	}
      elem = elem->next;
    }
#endif
}

//     rewrite subject into a newsubject
//     compare subject with oldsubject
//     substitue EQL in newsubject from corresponding member of oldsubject (or error if old not available)
//     replace subject and oldsubject with newsubject
success_t
pattern (container_context_t * CC, elem_t * subject)
{
  success_t rc;
  elem_t *pattern_acts, *nextpattern_act;

putc('\n',stdout);
putc('s',stdout);
putc(' ',stdout);
print_list(stdout,subject, 2, ' ');
putc('\n',stdout);
  pattern_acts = &(CC->pattern_acts);
  nextpattern_act = pattern_acts->u.list.first;
  while (nextpattern_act)
    {
      pattern_r (CC, subject, nextpattern_act->u.list.first);

putc('\n',stdout);
putc('p',stdout);
putc(' ',stdout);
print_list(stdout,subject, 2, ' ');
putc('\n',stdout);

      nextpattern_act = nextpattern_act->next;
    }

  rc = SUCCESS;
  return rc;
}
