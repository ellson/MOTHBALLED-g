#include <stdio.h>
#include <assert.h>

#include "grammar.h"
#include "inbuf.h"
#include "list.h"
#include "context.h"

//     flatten SUBJECT into a list of LEGS (or ENDPOINT or NODEID ?)
//     compare flattened list with prev_subject (already flattened)
//     substitue sameends (or error if no leg exists at index of EQL)
//     free prev_subject
//     save new (fully substitued and flattened) subject as prev_subject
success_t sameas(container_context_t *CC, elem_t *root) {
    success_t rc;
    elem_t *elem;

// FIXME -- implement

    free_list(&(CC->prev_subject));   // update prev_subject for same_end substitution
    elem = ref_list(SUBJECT, root);
    append_list(&(CC->prev_subject), elem);

    rc = SUCCESS;
    return rc;
}
