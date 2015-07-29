typedef struct {
    int nest;
} context_t;

#define OUT stdout
#define ERR stderr

char *get_name(char *p);
void print_string(unsigned char *frag, int flen);
char char_prop(unsigned char prop, char noprop);

typedef struct {
    void (*start_state_machine) (context_t *C);
    void (*indent) (context_t *C);
    void (*start_state) (context_t *C, char *p);
    void (*prop) (context_t *C, unsigned char prop);
    void (*string) (context_t *C, unsigned char *frag, int flen);
    void (*token) (context_t *C, unsigned char c);
    void (*end_state) (context_t *C, int rc);
    void (*term) (context_t *C);
    void (*end_state_machine) (context_t *C);
    void (*error) (context_t *C, char *message);
} emit_t;


#define emit_start_state_machine(C) \
    if (emit->start_state_machine) emit->start_state_machine(C)
#define emit_indent(C) \
    if (emit->indent) emit->indent(C)
#define emit_start_state(C, p) \
    if (emit->start_state) emit->start_state(C, p)
#define emit_prop(C, prop) \
    if (emit->prop) emit->prop(C, prop)
#define emit_string(C, ftag, flen) \
    if (emit->string) emit->string(C, ftag, flen)
#define emit_token(C, c) \
    if (emit->token) emit->token(C, c)
#define emit_end_state(C, rc) \
    if (emit->end_state) emit->end_state(C, rc)
#define emit_term(C) \
    if (emit->term) emit->term(C)
#define emit_end_state_machine(C) \
    if (emit->end_state_machine) emit->end_state_machine(C)
#define emit_error(C, message) \
    if (emit->error) emit->error(C, message)

extern emit_t *emit,
    *emit_trace_api,
    *emit_g_api;


