elem_t * ikea_store_open(context_t *C);
void ikea_store_snapshot (context_t *C);
void ikea_store_restore (context_t *C);
void ikea_store_close (context_t *C);

ikea_box_t* ikea_box_open( context_t * C, elem_t * name );
void ikea_box_append(ikea_box_t* ikea_box, unsigned char *data, size_t data_len);
void ikea_box_flush(ikea_box_t* ikea_box);
void ikea_box_close(ikea_box_t* ikea_box);
