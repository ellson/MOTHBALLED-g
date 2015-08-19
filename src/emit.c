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

char char_prop(unsigned char prop, char noprop)
{
	char c;

	if (prop & ALT) {
		c = '|';
	} else {
		if (prop & OPT) {
			if (prop & (SREP | REP)) {
				c = '*';
			} else {
				c = '?';
			}
		} else {
			if (prop & (SREP | REP)) {
				c = '+';
			} else {
				c = noprop;
			}
		}
	}
	return c;
}

static void print_list_r(FILE * chan, elem_t * list, char * sep)
{
	elem_t *elem;
	elemtype_t type;
	unsigned char *cp;
	int len;
	state_t si;

	assert(list);
	if (! (elem = list->u.list.first)) {
		return;
	}
	type = (elemtype_t) elem->type;
	switch (type) {
	case FRAGELEM:
		if ((state_t) list->state == DQT) {
			putc('"', chan);
		}
		while (elem) {
			cp = elem->u.frag.frag;
			len = elem->v.frag.len;
			assert(len > 0);
			if ((state_t) elem->state == BSL) {
				putc('\\', chan);
			}
			if ((state_t) elem->state == AST) {
				if (list->state == DQT) {
					putc('"', chan);
					putc('*', chan);
					putc('"', chan);
				} else {
					putc('*', chan);
				}
			}
			else while (len--) {
				putc(*cp++, chan);
			}
			elem = elem->next;
		}
		if ((state_t) list->state == DQT) {
			putc('"', chan);
		}
		break;
	case LISTELEM:
		si = (state_t) list->state;
		len = 0;
		while (elem) {
			if (len++ == 0) {
				*sep = 0;
				switch (si) {
				case EDGE:
					putc('<', chan);
					break;
				case OBJECT_LIST:
				case ENDPOINTSET:
					putc('(', chan);
					break;
				case ATTRIBUTES:
					putc('[', chan);
					break;
				case CONTAINER:
					putc('{', chan);
					break;
				case VALASSIGN:		  // FIXME - leaves spaces around '='
					putc('=', chan);
				default:
					*sep = ' ';
					break;
				}
			} else if (*sep) {
				putc(*sep, chan);
			}
			print_list_r(chan, elem, sep);	// recurse
			elem = elem->next;
		}
		*sep = 0;
		switch (si) {
		case EDGE:
			putc('>', chan);
			break;
		case OBJECT_LIST:
		case ENDPOINTSET:
			putc(')', chan);
			break;
		case ATTRIBUTES:
			putc(']', chan);
			break;
		case CONTAINER:
			putc('}', chan);
			break;
		default:
			*sep = ' ';
			break;
		}
		break;
	}
}

void print_subject(context_t * C, elem_t * list)
{
	FILE *chan;
	elem_t *elem;
    char sep = 0;

	assert(C);
	chan = C->out;

	assert(list);

	elem = list->u.list.first;  // skip ACT
	assert(elem);

	elem = elem->u.list.first;  // skip SUBJECT (which may have been
					// overloaded with a state of NODE or EDGE
	assert(elem);

	print_list_r(chan, elem, &sep);
}

void print_attributes(context_t * C, elem_t * list)
{
	FILE *chan;
	elem_t *elem;
    char sep = 0;

	assert(C);
	chan = C->out;

	assert(list);

	elem = list->u.list.first;
	if (elem) {
		putc('[', chan);
		print_list_r(chan, elem, &sep);
		putc(']', chan);
	}
}

void print_error(context_t * C, state_t si, char *message)
{
	unsigned char *p, c;

	fprintf(C->err, "\nError: %s ", message);
	print_len_frag(C->err, NAMEP(si));
	fprintf(C->err, "\n	  in \"%s\" line: %ld just before: \"",
		C->filename,
		(stat_lfcount ? stat_lfcount : stat_crcount) -
		C->linecount_at_start + 1);
	p = C->in;
	while ((c = *p++)) {
		if (c == '\n' || c == '\r')
			break;
		putc(c, C->err);
	}
	fprintf(C->err, "\"\n");
	exit(1);
}
