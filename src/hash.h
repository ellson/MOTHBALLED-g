elem_t *je_hash_bucket(context_t * C, unsigned long hash);
void je_hash_list(unsigned long *hash, elem_t *list);
void je_long_to_base64(char hashname[], unsigned long *phash);
success_t je_base64_to_long(char *b64string, unsigned long *phash);
