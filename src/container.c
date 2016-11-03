/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "content.h"
#include "container.h"

/**
 * parse G syntax input
 *
 * This parser recurses at two levels:
 *
 * parse() --> container() --> content() ---| -|  
 *           ^               ^  |           |  |
 *           |               |  -> doact()  |  |
 *           |               |              |  |
 *           |               --------<------|  |
 *           |                                 |
 *           ----------------<-----------------|
 *
 * The outer recursions are through nested containment.
 *
 * The inner recursions are through the grammar state_machine at a single
 * level of containment - maintained in the CONTENT context
 *
 * The top-level SESSION context is available to both and maintains the input state.
 *
 * @param PARSE context
 * @return success/fail
 */
success_t container(PARSE_t * PARSE)
{
    CONTENT_t container_context = { 0 };
    CONTENT_t * CONTENT = &container_context;
    success_t rc;
    TOKEN_t * TOKEN = (TOKEN_t *)PARSE;
    LIST_t * LIST = (LIST_t *)PARSE;
    elem_t *root = new_list(LIST, ACTIVITY);

    CONTENT->PARSE = PARSE;
    CONTENT->subject = new_list(LIST, SUBJECT);
    CONTENT->node_pattern_acts = new_list(LIST, 0);
    CONTENT->edge_pattern_acts = new_list(LIST, 0);
    CONTENT->ikea_box = ikea_box_open(PARSE->ikea_store, NULL);
    CONTENT->out = stdout;
    emit_start_activity(CONTENT);
    PARSE->containment++;            // containment nesting level
    PARSE->stat_containercount++;    // number of containers

    if ((rc = content(CONTENT, root, ACTIVITY, SREP, 0, 0)) == FAIL) {
        if (TOKEN->insi == NLL) {    // EOF is OK
            rc = SUCCESS;
        } else {
            token_error(TOKEN, TOKEN->state, "Parse error. Last good state was:");
        }
    }
    if (CONTENT->nodes) {
        PARSE->sep = ' ';
        print_tree(PARSE->out, CONTENT->nodes, &(PARSE->sep));
        putc('\n', PARSE->out);
    }
    if (CONTENT->edges) {
        PARSE->sep = ' ';
        print_tree(PARSE->out, CONTENT->edges, &(PARSE->sep));
        putc('\n', PARSE->out);
    }

// FIXME - don't forget to include NODE and EDGE patterns, after NODES and EDGES
//   (Paterns are in effect now, but may not have been at the creation of existing objects.)

    PARSE->containment--;
    emit_end_activity(CONTENT);

    ikea_box_close ( CONTENT->ikea_box );

    free_list(LIST, root);
    free_tree(LIST, CONTENT->nodes);
    free_tree(LIST, CONTENT->edges);
    free_list(LIST, CONTENT->subject);
    free_list(LIST, CONTENT->node_pattern_acts);
    free_list(LIST, CONTENT->edge_pattern_acts);

    if (LIST->stat_elemnow) {
        E();
        assert(LIST->stat_elemnow == 0);   // check for elem leaks
    }

    return rc;
}
