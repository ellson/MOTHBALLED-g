#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "grammar.h"
#include "inbuf.h"
#include "list.h"
#include "context.h"
#include "emit.h"
#include "info.h"

emit_t *emit;    // global

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

void g_append_token(container_context_t *CC, char **pos, char tok)
{
                        // ignore sep before
    *(*pos)++ = tok;    // copy token
    **pos = '\0';       // and replace terminating NULL
    CC->sep = 0;     // no sep required after tokens
}

void g_append_string(container_context_t *CC, char **pos, char *string)
{
    int len;

    if (CC->sep) *(*pos)++ = CC->sep; // sep before, if any
    len = sprintf(*pos,"%s",string);  // copy string
    // FIXME - deal with errors
    CC->sep = ' ';   // sep required after strings 
    *pos += len;
}

// FIXME - qstring should only quote if necessay, and should do escapes
void g_append_qstring(container_context_t *CC, char **pos, char *string)
{
    int len;

    if (CC->sep) *(*pos)++ = CC->sep; // sep before, if any
    len = sprintf(*pos,"\"%s\"",string);  // copy string
    // FIXME - deal with errors
    CC->sep = ' ';   // sep required after strings 
    *pos += len;
}

void g_append_ulong(container_context_t *CC, char **pos, unsigned long integer)
{
    int len;

    if (CC->sep) *(*pos)++ = CC->sep; // sep before, if any
    len = sprintf(*pos,"%lu",integer); // format integer to string
    // FIXME - deal with errors
    CC->sep = ' ';   // sep required after strings
    *pos += len;
}

// special case formatter for runtime
void g_append_runtime(container_context_t *CC, char **pos, unsigned long run_sec, unsigned long run_ns)
{
    int len;

    if (CC->sep) *(*pos)++ = CC->sep; // sep before, if any
    len = sprintf(*pos,"%lu.%09lu",run_sec, run_ns);
    // FIXME - deal with errors
    CC->sep = ' ';   // sep required after strings
    *pos += len;
}

static void put_token(container_context_t *CC, char tok)
{
    if (CC->style == SHELL_FRIENDLY_STYLE) {
        putc('\n', CC->out);
        putc(tok, CC->out);
        putc(' ', CC->out);
    } else {
        putc(tok, CC->out);
    }
    CC->sep = 0;
}

static void put_close_token(container_context_t *CC, char tok)
{
    if (CC->style == SHELL_FRIENDLY_STYLE) {
        putc('\n', CC->out);
        putc(tok, CC->out);
        putc('\n', CC->out);
    } else {
        putc(tok, CC->out);
    }
    CC->sep = 0;
}

static void print_list_r(container_context_t *CC, elem_t * list)
{
	elem_t *elem;
	elemtype_t type;
	int cnt;
	state_t liststate;

	assert(list);
	if (! (elem = list->u.list.first)) {
		return;
	}
	type = (elemtype_t) elem->type;
	liststate = (state_t) list->state;
	switch (type) {
	case FRAGELEM:
        print_frags(CC->out,liststate,elem,&(CC->sep));
		break;
	case LISTELEM:
		cnt = 0;
		while (elem) {
			if (cnt++ == 0) {
				switch (liststate) {
				case EDGE:
                    put_token(CC, '<');
					break;
				case OBJECT_LIST:
				case ENDPOINTSET:
                    put_token(CC, '(');
					break;
				case ATTRIBUTES:
                    put_token(CC, '[');
					break;
				case CONTAINER:
                    put_token(CC, '{');
					break;
				case VALASSIGN:
                    put_token(CC, '=');
                    break;
				default:
					break;
				}
			}
			print_list_r(CC, elem);	// recurse
			elem = elem->next;
		}
		switch (liststate) {
		case EDGE:
            put_close_token(CC, '>');
			break;
		case OBJECT_LIST:
		case ENDPOINTSET:
            put_close_token(CC, ')');
			break;
		case ATTRIBUTES:
            put_close_token(CC, ']');
			break;
		case CONTAINER:
            put_close_token(CC, '}');
			break;
		default:
			break;
		}
		break;
	}
}

void print_subject(container_context_t * CC, elem_t * subject)
{
	assert(subject);
	assert((state_t)subject->state == SUBJECT);

	print_list_r(CC, subject->u.list.first);
}

void print_attributes(container_context_t * CC, elem_t * attributes)
{
    if (attributes) {
	    assert((state_t)attributes->state == ATTRIBUTES);
	    print_list_r(CC, attributes);
    }
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
