/* vim:set shiftwidth=4 ts=8 expandtab: */

struct hashfile_s {
    hashfile_t *next;
    FILE *out;
    long hash;
};


elem_t * je_hash_bucket(context_t * C, unsigned long hash);
void je_hash_list(unsigned long *hash, elem_t *list);
void je_long_to_base64(char *b64string, const unsigned long *hash);
success_t je_base64_to_long(const char *b64string, unsigned long *hash);

