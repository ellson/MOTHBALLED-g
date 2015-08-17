char *get_name (char *p);
char char_prop (unsigned char prop, char noprop);
void print_subject (context_t * C, elem_t * list);
void print_attributes (context_t * C, elem_t * list);
void print_error (context_t * C, state_t si, char *message);

typedef struct
{
  void (*start_parse) (context_t * C);
  void (*end_parse) (context_t * C);

  void (*start_file) (context_t * C);
  void (*end_file) (context_t * C);

  void (*start_activity) (context_t * C);
  void (*end_activity) (context_t * C);

  void (*start_act) (context_t * C);
  void (*end_act) (context_t * C);

  void (*start_subject) (context_t * C);
  void (*end_subject) (context_t * C);

  void (*start_attributes) (context_t * C);
  void (*end_attributes) (context_t * C);

  void (*start_container) (context_t * C);
  void (*end_container) (context_t * C);

  void (*start_state) (context_t * C, char class, unsigned char prop,
		       int nest, int repc);
  void (*end_state) (context_t * C, char class, success_t rc, int nest,
		     int repc);

  void (*act) (context_t * C, elem_t * root);
  void (*subject) (context_t * C, elem_t * root);
  void (*attributes) (context_t * C, elem_t * root);

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

#define emit_start_act(C) \
    if (emit->start_act) {emit->start_act(C);}
#define emit_end_act(C) \
    if (emit->end_act) {emit->end_act(C);}

#define emit_start_subject(C) \
    if (emit->start_subject) {emit->start_subject(C);}
#define emit_end_subject(C) \
    if (emit->end_subject) {emit->end_subject(C);}

#define emit_start_attributes(C) \
    if (emit->start_attributes) {emit->start_attributes(C);}
#define emit_end_attributes(C) \
    if (emit->end_attributes) {emit->end_attributes(C);}

#define emit_start_container(C) \
    if (emit->start_container) {emit->start_container(C);}
#define emit_end_container(C) \
    if (emit->end_container) {emit->end_container(C);}

#define emit_start_state(C, class, prop, nest, repc) \
    if (emit->start_state) {emit->start_state(C, class, prop, nest, repc);}
#define emit_end_state(C, class, rc, nest, repc) \
    if (emit->end_state) {emit->end_state(C, class, rc, nest, repc);}

#define emit_act(C, root) \
    if (emit->act) {emit->act(C, root);}
#define emit_subject(C, root) \
    if (emit->subject) {emit->subject(C, root);}
#define emit_attributes(C, root) \
    if (emit->attributes) {emit->attributes(C, root);}

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
#undef emit_frag
#define emit_frag(C, len, frag)
//

extern emit_t *emit,
  *emit_t_api, *emit_t_api1, *emit_g_api, *emit_g_api1, *emit_g_api2;
