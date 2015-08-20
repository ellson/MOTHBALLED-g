char *get_name(char *p);
char char_prop(unsigned char prop, char noprop);
void print_subject(container_context_t * CC, elem_t * subject);
void print_attributes(container_context_t * CC, elem_t * attributes);
void print_error(context_t * CC, state_t si, char *message);

typedef struct {
    char *name;
	void (*start_parse) (context_t * C);
	void (*end_parse) (context_t * C);

	void (*start_file) (context_t * C);
	void (*end_file) (context_t * C);

	void (*start_activity) (context_t * C);
	void (*end_activity) (context_t * C);

	void (*start_act) (container_context_t * CC);
	void (*end_act) (container_context_t * CC);

	void (*start_subject) (container_context_t * CC);
	void (*end_subject) (container_context_t * CC);

	void (*start_state) (context_t * C, char class, unsigned char prop,
			     int nest, int repc);
	void (*end_state) (context_t * C, char class, success_t rc, int nest,
			   int repc);

	void (*act) (container_context_t * CC, elem_t * root);
	void (*subject) (container_context_t * CC, elem_t * root);
	void (*attributes) (container_context_t * CC, elem_t * root);

	void (*sep) (context_t * C);
	void (*token) (context_t * C, char token);
	void (*string) (context_t * C, elem_t * branch);
	void (*frag) (context_t * C, unsigned char len, unsigned char *frag);
	void (*error) (context_t * C, state_t si, char *message);
} emit_t;

#define emit_start_parse(C) \
    if (emit->start_parse) {emit->start_parse(C);}
#define emit_end_parse(C) \
    if (emit->end_parse) {emit->end_parse(C);}

#define emit_start_file(C) \
    if (emit->start_file) {emit->start_file(C);}
#define emit_end_file(C) \
    if (emit->end_file) {emit->end_file(C);}

#define emit_start_activity(C) \
    if (emit->start_activity) {emit->start_activity(C);}
#define emit_end_activity(C) \
    if (emit->end_activity) {emit->end_activity(C);}

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
//

extern emit_t *emit,
    *emit_t_api, *emit_t_api1, *emit_g_api, *emit_g_api1, *emit_g_api2;
