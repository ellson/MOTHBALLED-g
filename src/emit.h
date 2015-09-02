// emit styles
typedef enum {
	MINIMUM_SPACE_STYLE = 0,
	SHELL_FRIENDLY_STYLE = 1
} style_t;

struct emit_s {
    char *name;
	void (*initialize) (context_t * C);
	void (*finalize) (context_t * C);

	void (*start_file) (context_t * C);
	void (*end_file) (context_t * C);

	void (*start_activity) (container_context_t * CC);
	void (*end_activity) (container_context_t * CC);

	void (*start_act) (container_context_t * CC);
	void (*end_act) (container_context_t * CC);

	void (*start_subject) (container_context_t * CC);
	void (*end_subject) (container_context_t * CC);

	void (*start_state) (container_context_t * CC, char class, unsigned char prop, int nest, int repc);
	void (*end_state) (container_context_t * CC, char class, success_t rc, int nest, int repc);

	void (*act) (container_context_t * CC, elem_t * root);
	void (*subject) (container_context_t * CC, elem_t * root);
	void (*attributes) (container_context_t * CC, elem_t * root);

	void (*sep) (context_t * C);
	void (*token) (context_t * C, char token);
	void (*string) (context_t * C, elem_t * branch);
	void (*frag) (context_t * C, unsigned char len, unsigned char *frag);
	void (*error) (context_t * C, state_t si, char *message);
};

#define emit_initialize(C) \
    if (emit->initialize) {emit->initialize(C);}
#define emit_finalize(C) \
    if (emit->finalize) {emit->finalize(C);}

#define emit_start_file(C) \
    if (emit->start_file) {emit->start_file(C);}
#define emit_end_file(C) \
    if (emit->end_file) {emit->end_file(C);}

#define emit_start_activity(CC) \
    if (emit->start_activity) {emit->start_activity(CC);}
#define emit_end_activity(CC) \
    if (emit->end_activity) {emit->end_activity(CC);}

#define emit_start_act(CC) \
    if (emit->start_act) {emit->start_act(CC);}
#define emit_end_act(CC) \
    if (emit->end_act) {emit->end_act(CC);}

#define emit_start_subject(CC) \
    if (emit->start_subject) {emit->start_subject(CC);}
#define emit_end_subject(CC) \
    if (emit->end_subject) {emit->end_subject(CC);}

#define emit_start_state(C, class, prop, nest, repc) \
    if (emit->start_state) {emit->start_state(C, class, prop, nest, repc);}
#define emit_end_state(C, class, rc, nest, repc) \
    if (emit->end_state) {emit->end_state(C, class, rc, nest, repc);}
#define emit_act(CC, root) \
    if (emit->act) {emit->act(CC, root);}
#define emit_subject(CC, root) \
    if (emit->subject) {emit->subject(CC, root);}
#define emit_attributes(CC, root) \
    if (emit->attributes) {emit->attributes(CC, root);}

#define emit_sep(C) \
    if (emit->sep) {emit->sep(C);}
#define emit_token(C, token) \
    if (emit->token) {emit->token(C, token);}
#define emit_string(C, branch) \
    if (emit->string) {emit->string(C, branch);}
#define emit_frag(C, len, frag) \
    if (emit->frag) {emit->frag(C, len, frag);}
#define emit_error(C, si, message) \
    if (emit->error) {emit->error(C, si, message);}

// if we're not providing the function in any api,
//    then we can avoid the runtime cost of testing for it
#undef emit_start_act
#define emit_start_act(CC, len, frag)

#undef emit_end_act
#define emit_end_act(CC, len, frag)

#undef emit_start_subject
#define emit_start_subject(CC, len, frag)

#undef emit_end_subject
#define emit_end_subject(CC, len, frag)

#undef emit_frag
#define emit_frag(C, len, frag)

// emit.c
extern emit_t *emit;
extern emit_t g_api, g1_api, g2_api, t_api, t1_api, gv_api;
char je_char_prop(unsigned char prop, char noprop);
void je_append_token(context_t *C, char **pos, char tok);
void je_append_string(context_t *C, char **pos, char *string);
void je_append_ulong(context_t *C, char **pos, unsigned long integer);
void je_append_runtime(context_t *C, char **pos, unsigned long run_sec, unsigned long run_ns);
void je_emit_list(context_t * C, FILE * chan, elem_t * subject);
void je_emit_error(context_t * CC, state_t si, char *message);
