typedef struct {
    int nest;
} context_t;

char *get_name(char *p);

void emit_start_state_machine(context_t *C);
void emit_indent(context_t *C);
void emit_start_state(context_t *C, char *p);
void emit_prop(context_t *C, unsigned char prop);
void emit_string(context_t *C, unsigned char *frag, int flen);
void emit_token(context_t *C, unsigned char c);
void emit_end_state(context_t *C, int rc);
void emit_term(context_t *C);
void emit_end_state_machine(context_t *C);
void emit_error(context_t *C, char *message);

