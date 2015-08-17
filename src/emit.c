#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "grammar.h"
#include "inbuf.h"
#include "list.h"
#include "context.h"
#include "emit.h"
#include "stats.h"

emit_t *emit;

char
char_prop (unsigned char prop, char noprop)
{
  char c;

  if (prop & ALT)
    {
      c = '|';
    }
  else
    {
      if (prop & OPT)
	{
	  if (prop & (SREP | REP))
	    {
	      c = '*';
	    }
	  else
	    {
	      c = '?';
	    }
	}
      else
	{
	  if (prop & (SREP | REP))
	    {
	      c = '+';
	    }
	  else
	    {
	      c = noprop;
	    }
	}
    }
  return c;
}

static void
print_subject_r (FILE * chan, elem_t * list)
{
  elem_t *elem;
  elemtype_t type;
  unsigned char *cp;
  int len;

  assert (list);
  elem = list->u.list.first;
  assert (elem);

  type = (elemtype_t) elem->type;
  switch (type)
    {
    case FRAGELEM:
      if (list->state == DQT)
	{
	  putc ('"', chan);
	}
      while (elem)
	{
	  cp = elem->u.frag.frag;
	  len = elem->v.frag.len;
	  assert (len > 0);
	  if (elem->state == BSL)
	    {
	      putc ('\\', chan);
	    }
	  while (len--)
	    putc (*cp++, chan);
	  elem = elem->next;
	}
      if (list->state == DQT)
	{
	  putc ('"', chan);
	}
      break;
    case LISTELEM:
      len = 0;
      while (elem)
	{
	  if ((state_t) list->state == ENDPOINTSET)
	    {
	      if (len++)
		{
		  putc (' ', chan);
		}
	      else
		{
		  putc ('(', chan);
		}
	    }
	  else if ((state_t) list->state == EDGE)
	    {
	      if (len++)
		{
		  putc (' ', chan);
		}
	    }
	  print_subject_r (chan, elem);	// recurse
	  elem = elem->next;
	}
      if ((state_t) list->state == ENDPOINTSET)
	{
	  putc (')', chan);
	}
      break;
    }
}

void
print_subject (context_t * C, elem_t * list)
{
  FILE *chan;
  elem_t *elem;
  int len;

  assert (C);
  chan = C->out;

  assert (list);

  elem = list->u.list.first;
  assert (elem);

  if (list->u.list.first->next)
    {
      putc ('(', chan);
    }

  len = 0;
  while (elem)
    {
      if ((state_t) list->state == EDGE)
	{
	  putc ('<', chan);
	}
      else
	{
	  if (len++)
	    {
	      putc (' ', chan);
	    }
	}
      print_subject_r (chan, elem);
      if ((state_t) list->state == EDGE)
	{
	  putc ('>', chan);
	}
      elem = elem->next;
    }

  if (list->u.list.first->next)
    {
      putc (')', chan);
    }
  putc (' ', chan);
}

void
print_error (context_t * C, state_t si, char *message)
{
  unsigned char *p, c;

  fprintf (C->err, "\nError: %s ", message);
  print_len_frag (C->err, NAMEP (si));
  fprintf (C->err, "\n      in \"%s\" line: %ld just before: \"",
	   C->filename,
	   (stat_lfcount ? stat_lfcount : stat_crcount) -
	   C->linecount_at_start + 1);
  p = C->in;
  while ((c = *p++))
    {
      if (c == '\n' || c == '\r')
	break;
      putc (c, C->err);
    }
  fprintf (C->err, "\"\n");
  exit (1);
}
