ikea_t* ikea_open( context_t * C, elem_t * name );
void ikea_append(ikea_t* ikea, unsigned char *data, size_t data_len);
void ikea_flush(ikea_t* ikea);
void ikea_close(ikea_t* ikea);
void ikea_flush_all(context_t * C);
void ikea_close_all(context_t * C);
