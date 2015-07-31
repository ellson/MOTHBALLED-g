typedef struct {
    int nest;
} context_t;

#define OUT stdout
#define ERR stderr

char *get_name(char *p);
void print_string(unsigned char *len_frag);
void print_frag(FILE* chan, unsigned char len, unsigned char *frag);
char char_prop(unsigned char prop, char noprop);

typedef struct {
    void (*start_state_machine) (context_t *C);
    void (*sep) (context_t *C);
    void (*start_state) (context_t *C, char class, unsigned char prop, int nest, int repc);
    void (*string) (context_t *C, elem_t *leaves, int slen);
    void (*frag) (context_t *C, unsigned char len, unsigned char *frag);
    void (*tok) (context_t *C, char class, unsigned char len, unsigned char *frag);
    void (*end_state) (context_t *C, char class, int rc, int nest, int repc);
    void (*term) (context_t *C);
    void (*end_state_machine) (context_t *C);
    void (*error) (context_t *C, char *message);
} emit_t;


#define emit_start_state_machine(C) \
    if (emit->start_state_machine) {emit->start_state_machine(C);}
#define emit_sep(C) \
    if (emit->sep) {emit->sep(C);}
#define emit_start_state(C, class, prop, nest, repc) \
    if (emit->start_state) {emit->start_state(C, class, prop, nest, repc);}
#define emit_string(C, leaves, slen) \
    if (emit->string) {emit->string(C, leaves, slen);}
#define emit_frag(C, len, frag) \
    if (emit->frag) {emit->frag(C, len, frag);}
#define emit_tok(C, class, len, frag) \
    if (emit->tok) {emit->tok(C, class, len, frag);}
#define emit_end_state(C, class, rc, nest, repc) \
    if (emit->end_state) {emit->end_state(C, class, rc, nest, repc);}
#define emit_term(C) \
    if (emit->term) {emit->term(C);}
#define emit_end_state_machine(C) \
    if (emit->end_state_machine) {emit->end_state_machine(C);}
#define emit_error(C, message) \
    if (emit->error) {emit->error(C, message);}

extern emit_t *emit,
    *emit_t_api,
    *emit_g_api1,
    *emit_g_api2;


