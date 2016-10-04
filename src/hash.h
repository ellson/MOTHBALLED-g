/* vim:set shiftwidth=4 ts=8 expandtab: */

hash_elem_t * je_hash_bucket(CONTEXT_t * C, uint64_t hash);
void je_hash_list(uint64_t *hash, elem_t *list);
void je_long_to_base64(char *b64string, const uint64_t *hash);
success_t je_base64_to_long(const char *b64string, uint64_t *hash);

