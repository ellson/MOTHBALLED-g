#include "libje_private.h"

static void api_start_activity(container_context_t * CC)
{
	fprintf(CC->context->out, "graph {\n");
}

static void api_end_activity(container_context_t * CC)
{
	fprintf(CC->context->out, "\n}\n");
}

static void api_list(container_context_t * CC, elem_t *list)
{
    context_t *C = CC->context;

    je_emit_list(C, C->out, list);
}

emit_t gv_api = { "gv",
	/* api_initialize */ NULL,
	/* api_finalize */ NULL,

	/* api_start_file */ NULL,
	/* api_end_file */ NULL,

	/* api_start_activity */ api_start_activity,
	/* api_end_activity */ api_end_activity,

	/* api_start_act */ NULL,
	/* api_end_act */ NULL,

	/* api_start_subject */ NULL,
	/* api_end_subject */ NULL,

	/* api_start_state */ NULL,
	/* api_end_state */ NULL,

	/* api_act */ NULL,
	/* api_subject */ api_list,
	/* api_attributes */ api_list,

	/* api_sep */ NULL,
	/* api_token */ NULL,
	/* api_string */ NULL,

	/* api_frag */ NULL,

	/* api_error */ je_emit_error
};
