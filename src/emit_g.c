#include "libje_private.h"

emit_t g_api = { "g",
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

	/* api_act */ NULL,
	/* api_subject */ print_subject,
	/* api_attributes */ print_attributes,

	/* api_sep */ NULL,
	/* api_token */ NULL,
	/* api_string */ NULL,

	/* api_frag */ NULL,

	/* api_error */ print_error
};

static void api1_end_activity(container_context_t * CC)
{
	putc('\n', CC->context->out);
}

static void api1_sep(context_t * C)
{
	putc(' ', C->out);
}

static void api1_string(context_t *C, elem_t * branch)
{
    char sep;

    sep = 0;
	print_list(C->out, branch, -1, &sep);
}

static void api1_token(context_t *C, char token)
{
	putc(token, C->out);
}

emit_t g1_api = { "g1",
	/* api_initialize */ NULL,
	/* api_finalize */ NULL,

	/* api_start_file */ NULL,
	/* api_end_file */ NULL,

	/* api_start_activity */ NULL,
	/* api_end_activity */ api1_end_activity,

	/* api_start_act */ NULL,
	/* api_end_act */ NULL,

	/* api_start_subject */ NULL,
	/* api_end_subject */ NULL,

	/* api_start_state */ NULL,
	/* api_end_state */ NULL,

	/* api_act */ NULL,
	/* api_subject */ NULL,
	/* api_attributes */ NULL,

	/* api_sep */ api1_sep,
	/* api_token */ api1_token,
	/* api_string */ api1_string,

	/* api_frag */ NULL,

	/* api_error */ print_error
};

static void api2_token(context_t * C, char token)
{
	putc('\n', C->out);
	putc(token, C->out);
	putc(' ', C->out);
}

emit_t g2_api = { "g2",
	/* api_initialize */ NULL,
	/* api_finalize */ NULL,

	/* api_start_file */ NULL,
	/* api_end_file */ NULL,

	/* api_start_activity */ NULL,
	/* api_end_activity */ api1_end_activity,

	/* api_start_act */ NULL,
	/* api_end_act */ NULL,

	/* api_start_subject */ NULL,
	/* api_end_subject */ NULL,

	/* api_start_state */ NULL,
	/* api_end_state */ NULL,

	/* api_act */ NULL,
	/* api_subject */ NULL,
	/* api_attributes */ NULL,

	/* api_sep */ api1_sep,
	/* api_token */ api2_token,
	/* api_string */ api1_string,

	/* api_frag */ NULL,

	/* api_error */ print_error
};

emit_t *emit_g_api = &g_api;
emit_t *emit_g_api1 = &g1_api;
emit_t *emit_g_api2 = &g2_api;
