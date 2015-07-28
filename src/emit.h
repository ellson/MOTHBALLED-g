typedef struct {
	char *path;
} context_t;

void emit_start_act(context_t *context);
void emit_start_state(context_t *context, char *p);

