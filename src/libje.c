#include "libje_private.h"

context_t *je_initialize(void)
{
    context_t *C;

    if (! (C = calloc(1, sizeof(context_t)))) {
        perror("Error - calloc(): ");
        exit(EXIT_FAILURE);
    }

    emit_initialize(C);
    return C;
}

void je_finalize(context_t *C)
{
   emit_finalize(C);

   // free context
   free(C);
}
