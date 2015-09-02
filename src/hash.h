struct hashfile_s {
    hashfile_t *next;
    FILE *out;
    long hash;
};

elem_t * je_hash_bucket(context_t * C, unsigned long hash);
void je_hash_list(unsigned long *hash, elem_t *list);
char * je_long_to_base64(unsigned long *phash);
success_t je_base64_to_long(char *b64string, unsigned long *phash);

