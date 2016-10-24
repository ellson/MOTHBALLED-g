/* vim:set shiftwidth=4 ts=8 expandtab: */

uint16_t print_len_frag(FILE * chan, unsigned char *len_frag);
void print_frags(FILE * chan, state_t state, elem_t * elem, char *sep);
void print_list(FILE * chan, elem_t * list, int indent, char *sep);
