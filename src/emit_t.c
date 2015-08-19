#include <stdio.h>
#include <stdlib.h>

#include "grammar.h"
#include "inbuf.h"
#include "list.h"
#include "context.h"
#include "emit.h"

static void api_start_activity(context_t * C)
{
	if (C->containment == 0) {
		fprintf(C->out,
			"// |-- on entry, '|' alt, '.' one, '?' zero or one, '*' zero or more, '+' one or more\n");
		fprintf(C->out, "// |-- on exit, '0' success, '1' fail\n");
		fprintf(C->out, "//     |-- state number\n");
		fprintf(C->out, "//         |-- nesting\n");
		fprintf(C->out, "//             |-- iteration\n");
		fprintf(C->out, "//                 |-- state name\n");
		fprintf(C->out,
			"//                              |-- string (if present)\n");
	}
}

static void
api_start_state(context_t * C, char class, unsigned char prop, int nest,
		int repc)
{
	fprintf(C->out, "\n   ");
	putc(char_prop(prop, '.'), C->out);
	fprintf(C->out, "%4d%4d%4d   ", class, nest, repc);
	print_len_frag(C->out, NAMEP(class));
}

static void api_string(context_t * C, elem_t * branch)
{
	fprintf(C->out, "\t\"");
	print_list(C->out, branch, -1, 0);
	putc('"', C->out);
}

static void api_token(context_t * C, char token)
{
	fprintf(C->out, "\t\t'%c'", token);
}

static void
api_end_state(context_t * C, char class, success_t rc, int nest, int repc)
{
	fprintf(C->out, "\n   %d%4d%4d%4d   ", rc, class, nest, repc);
	print_len_frag(C->out, NAMEP(class));
}

static void api_end_activity(context_t * C)
{
	if (C->containment == 0) {
		putc('\n', C->out);
	}
}

static void api_end_parse(context_t * C)
{
	putc('\n', C->out);
}

static emit_t api = {
	/* api_start_parse */ NULL,
	/* api_end_parse */ api_end_parse,

	/* api_start_file */ NULL,
	/* api_end_file */ NULL,

	/* api_start_activity */ api_start_activity,
	/* api_end_activity */ api_end_activity,

	/* api_start_act */ NULL,
	/* api_end_act */ NULL,

	/* api_start_subject */ NULL,
	/* api_end_subject */ NULL,

	/* api_start_state */ api_start_state,
	/* api_end_state */ api_end_state,

	/* api_act */ NULL,
	/* api_subject */ NULL,
	/* api_attributes */ NULL,

	/* api_sep */ NULL,
	/* api_token */ api_token,
	/* api_string */ api_string,

	/* api_frag */ NULL,

	/* api_error */ print_error
};

static void api1_act(context_t * C, elem_t * tree)
{
	fprintf(C->out, "%3d ACT", C->containment);
	print_list(C->out, tree, 7, ' ');
}

static emit_t api1 = {
	/* api_start_parse */ NULL,
	/* api_end_parse */ api_end_parse,

	/* api_start_file */ NULL,
	/* api_end_file */ NULL,

	/* api_start_activity */ NULL,
	/* api_end_activity */ api_end_activity,

	/* api_start_act */ NULL,
	/* api_end_act */ NULL,

	/* api_start_subject */ NULL,
	/* api_end_subject */ NULL,

	/* api_start_state */ NULL,
	/* api_end_state */ NULL,

	/* api_act */ api1_act,
	/* api_subject */ NULL,
	/* api_attributes */ NULL,

	/* api_sep */ NULL,
	/* api_token */ NULL,
	/* api_string */ NULL,

	/* api_frag */ NULL,

	/* api_error */ print_error
};

emit_t *emit_t_api = &api;
emit_t *emit_t_api1 = &api1;
