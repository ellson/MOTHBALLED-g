elem_t * ikea_store_open(context_t *C);
void ikea_store_snapshot (context_t *C);
void ikea_store_restore (context_t *C);
void ikea_store_close (context_t *C);

ikea_box_t* ikea_open( context_t * C, elem_t * name );
void ikea_append(ikea_box_t* ikea, unsigned char *data, size_t data_len);
void ikea_flush(ikea_box_t* ikea);
void ikea_close(ikea_box_t* ikea);
