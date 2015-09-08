#include "libje_private.h"

static emit_t *emitters[] =
    {&g_api, &g1_api, &g2_api, &t_api, &t1_api, &gv_api};

static void api_act(container_context_t * CC, elem_t *list)
{
    context_t *C = CC->context;
    elem_t *elem;

    // after dispatch() there can be mutiple ACTs in the list
    elem = list->u.list.first;
    while (elem) {
        C->sep = 0;         // suppress space before (because preceded by BOF or NL)
        je_emit_list(CC->context, CC->out, elem);
        putc('\n', CC->out);   // NL after
        elem = elem->next;
    }
}

// this is default emitter used for writing to the file-per-container
// stored in the temp directory
static emit_t null_api = { "null",
	/* api_initialize */ NULL,
	/* api_finalize */ NULL,

	/* api_start_file */ NULL,
	/* api_end_file */ NULL,

	/* api_start_activity */ NULL,
	/* api_end_activity */ NULL,

	/* api_start_act */ NULL,
	/* api_end_act */ NULL,

	/* api_start_subject */ NULL,
	/* api_end_subject */ NULL,

	/* api_start_state */ NULL,
	/* api_end_state */ NULL,

	/* api_act */ api_act,
	/* api_subject */ NULL,
	/* api_attributes */ NULL,

	/* api_sep */ NULL,
	/* api_token */ NULL,
	/* api_string */ NULL,

	/* api_frag */ NULL,

	/* api_error */ je_emit_error
};

emit_t *emit = &null_api;

success_t je_select_emitter(char *name)
{
    size_t i;
    emit_t *ep;

    for (i = 0; i < sizeof(emitters)/sizeof(emitters[0]); i++) {
        ep = emitters[i];
        if (strcmp(name,ep->name) == 0) {
             emit = ep;
             return SUCCESS;
        }
    }
    return FAIL;
}

char je_char_prop(unsigned char prop, char noprop)
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

void je_append_token(context_t *C, char **pos, char tok)
{
    // FIXME - check available buffer space
                        // ignore sep before
    *(*pos)++ = (unsigned char)tok;    // copy token
    **pos = '\0';       // and replace terminating NULL
    C->sep = 0;        // no sep required after tokens
}

void je_append_string(context_t *C, char **pos, char *string)
{
    int len;

    // FIXME - check available buffer space
    if (C->sep) {
        *(*pos)++ = C->sep; // sep before, if any
    }
    len = sprintf(*pos,"%s",string);  // copy string
    if (len < 0) {
        perror("Error - sprintf(): ");
        exit(EXIT_FAILURE);
    }
    C->sep = ' ';      // sep required after strings 
    *pos += len;
}

void je_append_ulong(context_t *C, char **pos, unsigned long integer)
{
    int len;

    // FIXME - check available buffer space
    if (C->sep) {
        *(*pos)++ = C->sep; // sep before, if any
    }
    len = sprintf(*pos,"%lu",integer); // format integer to string
    if (len < 0) {
        perror("Error - sprintf(): ");
        exit(EXIT_FAILURE);
    }
    C->sep = ' ';      // sep required after strings
    *pos += len;
}

// special case formatter for runtime
void je_append_runtime(context_t *C, char **pos,
        unsigned long run_sec, unsigned long run_ns)
{
    int len;

    // FIXME - check available buffer space
    if (C->sep) *(*pos)++ = C->sep; // sep before, if any
    len = sprintf(*pos,"%lu.%09lu",run_sec, run_ns);
    if (len < 0) {
        perror("Error - sprintf(): ");
        exit(EXIT_FAILURE);
    }
    C->sep = ' ';   // sep required after strings
    *pos += len;
}

static void je_emit_token(context_t *C, FILE *chan, char tok)
{
    if (C->style == SHELL_FRIENDLY_STYLE) {
        putc('\n', chan);
        putc(tok, chan);
        putc(' ', chan);
    } else {
        putc(tok, chan);
    }
    C->sep = 0;
}

static void je_emit_close_token(context_t *C, FILE *chan, char tok)
{
    if (C->style == SHELL_FRIENDLY_STYLE) {
        putc('\n', chan);
        putc(tok, chan);
        putc('\n', chan);
    } else {
        putc(tok, chan);
    }
    C->sep = 0;
}

void je_emit_list(context_t *C, FILE *chan, elem_t * list)
{
	elem_t *elem;
	elemtype_t type;
	int cnt;
	state_t liststate;

	assert(list);
	liststate = (state_t) list->state;
	if (! (elem = list->u.list.first)) {
		switch (liststate) {
		case QRY:
            je_emit_token(C, chan, '?');
			break;
		case TLD:
            je_emit_token(C, chan, '~');
			break;
		default:
		    break;
		}
		return;
	}
	type = (elemtype_t) elem->type;
	switch (type) {
	case FRAGELEM:
        print_frags(chan, liststate, elem, &(C->sep));
		break;
	case LISTELEM:
		cnt = 0;
		while (elem) {
			if (cnt++ == 0) {
				switch (liststate) {
				case EDGE:
                    je_emit_token(C, chan, '<');
					break;
				case OBJECT_LIST:
				case ENDPOINTSET:
                    je_emit_token(C, chan, '(');
					break;
				case ATTRIBUTES:
                    je_emit_token(C, chan, '[');
					break;
				case CONTAINER:
                    je_emit_token(C, chan, '{');
					break;
				case VALASSIGN:
                    je_emit_token(C, chan, '=');
                    break;
		        case CHILD:
                    je_emit_token(C, chan, '/');
			        break;
				default:
					break;
				}
			}
			je_emit_list(C, chan, elem);	// recurse
			elem = elem->next;
		}
		switch (liststate) {
		case EDGE:
            je_emit_close_token(C, chan, '>');
			break;
		case OBJECT_LIST:
		case ENDPOINTSET:
            je_emit_close_token(C, chan, ')');
			break;
		case ATTRIBUTES:
            je_emit_close_token(C, chan, ']');
			break;
		case CONTAINER:
            je_emit_close_token(C, chan, '}');
			break;
		default:
			break;
		}
		break;
	case HASHELEM:
        assert(0);  // should not be here
		break;
	}
}

void je_emit_error(context_t * C, state_t si, char *message)
{
	unsigned char *p, c;

	fprintf(stderr, "Error: %s ", message);
	print_len_frag(stderr, NAMEP(si));
	fprintf(stderr, "\n	  in \"%s\" line: %ld just before: \"",
		C->filename, (C->stat_lfcount ?
            C->stat_lfcount :
            C->stat_crcount) - C->linecount_at_start + 1);
	p = C->in;
	while ((c = *p++)) {
		if (c == '\n' || c == '\r') {
			break;
        }
		putc(c, stderr);
	}
	fprintf(stderr, "\"\n");
	exit(EXIT_FAILURE);
}
