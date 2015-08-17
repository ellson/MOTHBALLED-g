#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "grammar.h"
#include "inbuf.h"
#include "list.h"
#include "context.h"
#include "stats.h"
#include "emit.h"
#include "parse.h"
#include "token.h"
#include "sameas.h"

// This parser recurses at two levels:
//
//     parse() ----> parse_activity(C) ----> parse_r(CC) -| -|  
//                   ^                       ^            |  |
//                   |                       |            |  |
//                   |                       ------<-------  |
//                   |                                       |
//                   -----------------<-----------------------
//
// The inner recursions is through the grammar state_machine at a single
// level of containment - maintained in container_context (CC)
//
// The outer recursion is through nested containment.
// The top-level context (C) is available to both and maintains the input state.

static success_t
more_rep (context_t * C, unsigned char prop)
{
  state_t ei, bi;

  if (!(prop & (REP | SREP)))
    return FAIL;

  ei = C->ei;
  if (ei == RPN || ei == RAN || ei == RBR || ei == RBE)
    {
      return FAIL;		// no more repetitions
    }
  bi = C->bi;
  if (bi == RPN || bi == RAN || bi == RBR || bi == RBE
      || (ei != ABC && ei != AST && ei != DQT))
    {
      return SUCCESS;		// more repetitions, but additional WS sep is optional
    }
  if (prop & SREP)
    {
      emit_sep (C);		// sep is non-optional, emit the minimal sep
    }
  return SUCCESS;		// more repetitions
}

static success_t parse_activity (context_t * C);	// forward declaration for recursion

static success_t
parse_r (container_context_t * CC, elem_t * root,
	 state_t si, unsigned char prop, int nest, int repc)
{
  unsigned char nprop;
  char so;			// offset to next state, signed
  state_t ti, ni;
  success_t rc;
  elem_t *elem;
  elem_t branch = { 0 };
  context_t *C;
  static unsigned char nullstring[] = { '\0' };

  C = CC->context;

  rc = SUCCESS;
  emit_start_state (C, si, prop, nest, repc);
  branch.state = si;

  nest++;
  assert (nest >= 0);		// catch overflows

  if (!C->inbuf)
    {				// state_machine just started
      C->bi = WS;		// pretend preceeded by WS
      // to satisfy toplevel SREP or REP
      // (Note, first REP of a REP sequence *can* be preceeded by WS,
      //      just not the rest of the REPs. )
      C->in = nullstring;	// fake it;
      C->insi = NLL;		// pretend last input was the EOF of a prior file.
    }


  // Entering state
  C->state = si;		// record of last state entered, for error messages.

  // deal with "terminal" states: Whitespace, Tokens, and Contained activity, Strings

  C->ei = C->insi;		// the char class that ended the last token
  // Whitespace
  if ((rc = parse_whitespace (C)) == FAIL)
    {
      goto done;		// EOF during whitespace
    }
  // Special character tokens
  if (si == C->insi)
    {				// single character terminals matching state_machine expectation
      C->bi = C->insi;
      rc = parse_token (C);
      C->ei = C->insi;
      goto done;
    }
  switch (si)
    {
      // Recursion into Contained activity
    case ACTIVITY:
      if (C->bi == LBE)
	{			// if not top-level of containment
	  C->bi = NLL;
	  rc = parse_activity (C);	// recursively process contained ACTIVITY in to its own root
	  C->bi = C->insi;	// the char class that terminates the ACTIVITY
	  goto done;
	}
      break;
      // Strings
    case STRING:
      rc = parse_string (C, &branch);
      C->bi = C->insi;		// the char class that terminates the STRING
      goto done;
      break;

      // the remainder of the switch() is just state initialization and emit hooks;

    case ACT:
      emit_start_act (C);
      break;
    case SUBJECT:
      emit_start_subject (C);
      C->has_ast = 0;
      C->is_pattern = 0;
      break;
    default:
      break;
    }

  // If it wasn't a terminal state, then using the state_machine,  
  // iterate through alts or sequences, and then recursively process next the state

  rc = FAIL;			// init rc to FAIL in case no ALT is satisfied
  ti = si;
  while ((so = state_machine[ti]))
    {				// iterate over ALTs or sequences
      nprop = state_props[ti];	// get the props for the transition from the current state (OPT, ALT, REP etc)
      // at this point, ni is a signed, non-zero offset to the next state
      ni = ti + so;		// we get to the next state by adding the offset to the current state.
      if (nprop & ALT)
	{			// look for ALT
	  if ((rc = parse_r (CC, &branch, ni, nprop, nest, 0)) == SUCCESS)
	    {
	      break;		// ALT satisfied
	    }
	  // we failed an ALT so continue iteration to try next ALT
	}
      else
	{			// else it is a sequence
	  repc = 0;
	  if (nprop & OPT)
	    {			// optional
	      if ((parse_r (CC, &branch, ni, nprop, nest, repc++)) == SUCCESS)
		{
		  while (more_rep (C, nprop) == SUCCESS)
		    {
		      if (parse_r (CC, &branch, ni, nprop, nest, repc++) ==
			  FAIL)
			{
			  break;
			}
		    }
		}
	    }
	  else
	    {			// else not OPTional
	      if ((rc =
		   parse_r (CC, &branch, ni, nprop, nest, repc++)) == FAIL)
		{
		  break;
		}
	      // A 1-or-more repetition is successful if the first one was a success
	      while (more_rep (C, nprop) == SUCCESS)
		{
		  if ((rc =
		       parse_r (CC, &branch, ni, nprop, nest,
				repc++)) == FAIL)
		    {
		      break;
		    }
		}
	    }
	}
      ti++;			// next ALT (if not yet satisfied), or next sequence item
    }

  // Any subtree rewrites or emits before adding branch to root in the state exit processing
  if (rc == SUCCESS)
    {
      switch (si)
	{
	case ACT:
	  if (C->is_pattern)
	    {	// flag was set by SUBJECT.  save SUBJECT ATTRIBUTES and CONTAINER
		//     in a list of pattern_acts
	      stat_patterncount++;
	      elem = move_list (si, &branch);	// moved completely, so no regular ACT remains
	      append_list (&(CC->pattern_acts), elem);
	    }
	  else
	    {
	      stat_actcount++;
	    }
	  emit_act (C, &branch);
	  break;
	case SUBJECT:
	  if (C->has_ast)
	    {
	      C->is_pattern = 1;
	    }
	  else
	    {
	      // Perform EQL "same as in subject of previous ACT" substitutions
	      if ((rc = sameas (CC, &branch)) == FAIL)
		{
		  break;
		}
	    }
            emit_subject(C, &branch); // emit rewritten subject
	  break;
	case ATTRIBUTES:
	  emit_attributes (C, &branch);
	  break;
	default:
	  break;
	}
    }

  // State exit processing

done:
  if (rc == SUCCESS)
    {
      if (branch.u.list.first != NULL || si == EQL)
	{			// mostly ignore empty lists
	  elem = move_list (si, &branch);
	  append_list (root, elem);
	  switch (si)
	    {
	    case ACT:
	      free_list (root);	// the parser is finished with entire ACT at this point. (emits happened earlier)
	      // subtrees of the output may be retained by ref_counted "copies" for:
	      //    patterns 
	      //    previous_subject (for sameends)
	      //    nodes and edges - for rendering
	      emit_end_act (C);
	      break;
	    case SUBJECT:
	      emit_end_subject (C);
	      break;
	    default:
	      break;
	    }
	}
    }
  nest--;
  assert (nest >= 0);
  emit_end_state (C, si, rc, nest, repc);

  return rc;
}

static success_t
parse_activity (context_t * C)
{
  success_t rc;
  elem_t root = { 0 };		// the output parse tree
  container_context_t container_context = { 0 };

  container_context.context = C;
  container_context.out = stdout;
  container_context.err = stderr;

  if(C->containment)
    {
      emit_start_container (C);
    }
  C->containment++;
  stat_containercount++;
  emit_start_activity (C);

  if ((rc =
       parse_r (&container_context, &root, ACTIVITY, SREP, 0, 0)) != SUCCESS)
    {
      if (C->insi == NLL)
	{			// EOF is OK
	  rc = SUCCESS;
	}
      else
	{
	  emit_error (C, C->state, "Parse error. Last good state was:");
	}
    }

  free_list (&container_context.prev_subject);
  free_list (&container_context.pattern_acts);

  emit_end_activity (C);
  C->containment++;
  if(C->containment)
    {
      emit_end_container (C);
    }
  return rc;
}

success_t
parse (int *pargc, char *argv[], FILE * out, FILE * err)
{
  success_t rc;
  context_t context = { 0 };	// the input context

  context.pargc = pargc;
  context.argv = argv;
  context.out = out;
  context.err = err;

  emit_start_parse (&context);
  rc = parse_activity (&context);
  emit_end_parse (&context);

  return rc;
}
