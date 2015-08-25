#include <stdlib.h>

#include "grammar.h"
#include "inbuf.h"
#include "list.h"
#include "context.h"
#include "emit.h"

static void api_start_activity(container_context_t * CC)
{
	if (CC->context->containment == 0) {
		fprintf(CC->out, "// |-- on entry, '|' alt, '.' one, '?' zero or one, '*' zero or more, '+' one or more\n");
		fprintf(CC->out, "// |-- on exit, '0' success, '1' fail\n");
		fprintf(CC->out, "//     |-- state number\n");
		fprintf(CC->out, "//         |-- nesting\n");
		fprintf(CC->out, "//             |-- iteration\n");
		fprintf(CC->out, "//                 |-- state name\n");
		fprintf(CC->out, "//                              |-- string (if present)\n");
	}
}

static void
api_start_state(container_context_t * CC, char class, unsigned char prop, int nest, int repc)
{
	fprintf(CC->out, "\n   ");
	putc(char_prop(prop, '.'), CC->out);
	fprintf(CC->out, "%4d%4d%4d   ", class, nest, repc);
	print_len_frag(CC->out, NAMEP(class));
}

static void api_string(container_context_t * CC, elem_t * branch)
{
    char sep;

    sep = 0;
	putc('\t', CC->out);
	print_list(CC->out, branch, -1, &sep);
}

static void api_token(container_context_t * CC, char token)
{
	fprintf(CC->out, "\t\t%c", token);
}

static void
api_end_state(container_context_t * CC, char class, success_t rc, int nest, int repc)
{
	fprintf(CC->out, "\n   %d%4d%4d%4d   ", rc, class, nest, repc);
	print_len_frag(CC->out, NAMEP(class));
}

static void api_end_activity(container_context_t * CC)
{
	putc('\n', CC->out);
}

emit_t t_api = { "t",
	/* api_start_parse */ NULL,
	/* api_end_parse */ NULL,

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

static void api1_act(container_context_t * CC, elem_t * tree)
{
    char sep;

    sep = ' ';
	fprintf(CC->out, "%3d ACT", CC->context->containment);
	print_list(CC->out, tree, 7, &sep);
}

emit_t t1_api = { "t1",
	/* api_start_parse */ NULL,
	/* api_end_parse */ NULL,

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

emit_t *emit_t_api = &t_api;
emit_t *emit_t_api1 = &t1_api;
