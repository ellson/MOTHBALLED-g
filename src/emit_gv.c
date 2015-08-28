#include "libje_private.h"

static void api_start_activity(container_context_t * CC)
{
	fprintf(CC->out, "graph {\n");
}

static void api_end_activity(container_context_t * CC)
{
	fprintf(CC->out, "\n}\n");
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
	/* api_subject */ print_subject,
	/* api_attributes */ print_attributes,

	/* api_sep */ NULL,
	/* api_token */ NULL,
	/* api_string */ NULL,

	/* api_frag */ NULL,

	/* api_error */ print_error
};

emit_t *emit_gv_api = &gv_api;
