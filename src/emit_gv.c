#include <stdio.h>
#include <stdlib.h>

#include "grammar.h"
#include "inbuf.h"
#include "list.h"
#include "context.h"
#include "emit.h"

static void api_start_parse(context_t *C)
{
	fprintf(C->out, "graph {\n");
}

static void api_end_parse(context_t *C)
{
	fprintf(C->out, "\n}\n");
}

emit_t gv_api = { "gv",
	/* api_start_parse */ api_start_parse,
	/* api_end_parse */ api_end_parse,

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

emit_t *emit_gv_api = &gv_api;
