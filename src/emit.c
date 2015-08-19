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

static void print_list_r(container_context_t *CC, elem_t * list)
{
	elem_t *elem;
	elemtype_t type;
	unsigned char *cp;
	int len;
	state_t si;
    FILE *chan;
    char *sep;

	assert(list);
	if (! (elem = list->u.list.first)) {
		return;
	}
    chan = CC->context->out;
    sep = &(CC->sep);
	type = (elemtype_t) elem->type;
	switch (type) {
	case FRAGELEM:
        if (*sep) {
			putc(*sep, chan);
        }
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
				if ((state_t) list->state == DQT) {
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
        *sep = ' ';
		break;
	case LISTELEM:
		si = (state_t) list->state;
		len = 0;
		while (elem) {
			if (len++ == 0) {
				switch (si) {
				case EDGE:
					putc('<', chan);
		            *sep = 0;
					break;
				case OBJECT_LIST:
				case ENDPOINTSET:
					putc('(', chan);
		            *sep = 0;
					break;
				case ATTRIBUTES:
					putc('[', chan);
		            *sep = 0;
					break;
				case CONTAINER:
					putc('{', chan);
		            *sep = 0;
					break;
				case VALASSIGN:		  // FIXME - leaves spaces around '='
					putc('=', chan);
		            *sep = 0;
                    break;
				default:
					break;
				}
			}
			print_list_r(CC, elem);	// recurse
			elem = elem->next;
		}
		switch (si) {
		case EDGE:
			putc('>', chan);
		    *sep = 0;
			break;
		case OBJECT_LIST:
		case ENDPOINTSET:
			putc(')', chan);
		    *sep = 0;
			break;
		case ATTRIBUTES:
			putc(']', chan);
		    *sep = 0;
			break;
		case CONTAINER:
			putc('}', chan);
		    *sep = 0;
			break;
		default:
			break;
		}
		break;
	}
}

void print_subject(container_context_t * CC, elem_t * list)
{
	elem_t *elem;

	assert(list);

	elem = list->u.list.first;  // skip ACT
	assert(elem);

	elem = elem->u.list.first;  // skip SUBJECT (which may have been
					// overloaded with a state of NODE or EDGE
	assert(elem);

	print_list_r(CC, elem);
}

void print_attributes(container_context_t * CC, elem_t * list)
{
	assert(list);

	print_list_r(CC, list);
}

void print_error(context_t * C, state_t si, char *message)
{
	unsigned char *p, c;

	fprintf(C->err, "\nError: %s ", message);
	print_len_frag(C->err, NAMEP(si));
	fprintf(C->err, "\n	  in \"%s\" line: %ld just before: \"",
		C->filename, (stat_lfcount ? stat_lfcount : stat_crcount) - C->linecount_at_start + 1);
	p = C->in;
	while ((c = *p++)) {
		if (c == '\n' || c == '\r')
			break;
		putc(c, C->err);
	}
	fprintf(C->err, "\"\n");
	exit(1);
}
