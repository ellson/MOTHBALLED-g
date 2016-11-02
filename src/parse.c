/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "parse.h"

// forward declarations

static success_t
parse_content_r(PARSE_t * PARSE, elem_t * subject);

static success_t
parse_list_r(CONTENT_t * CONTENT, elem_t * root, state_t si, unsigned char prop, int nest, int repc);

static success_t
parse_more_rep(PARSE_t * PARSE, unsigned char prop);

/**
 * parse G syntax input
 *
 * This parser recurses at two levels:
 *
 * parse() --> parse_content_r() --> parse_list_r() --| -|  
 *           ^                     ^     |            |  |
 *           |                     |     -> doact()   |  |
 *           |                     |                  |  |
 *           |                     ----------<--------|  |
 *           |                                           |
 *           ----------------------<---------------------|
 *
 * The outer recursions are through nested containment.
 *
 * The inner recursions are through the grammar state_machine at a single
 * level of containment - maintained in container_context (CONTENT)
 *
 * The top-level context (PARSE) is available to both and maintains the input state.
 *
 * @param PARSE context
 * @return success/fail
 */
success_t parse(PARSE_t * PARSE)
{
    success_t rc;

    rc = parse_content_r(PARSE, NULL);
    return rc;
}

static success_t parse_content_r(PARSE_t * PARSE, elem_t * subject)
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

    if ((rc = parse_list_r(CONTENT, root, ACTIVITY, SREP, 0, 0)) == FAIL) {
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

/** 
 * iterate and recurse through state-machine at a single level of containment
 *
 *  @param CONTENT container context
 *  @param root - the parent's branch that we are adding to
 *  @param si input state
 *  @param prop grammar properties
 *  @param nest recursion nesting level (containment level)
 *  @param repc sequence member counter
 *  @return SUCCESS or FAIL
 */
static success_t
parse_list_r(CONTENT_t * CONTENT, elem_t *root, state_t si, unsigned char prop, int nest, int repc)
{
    PARSE_t * PARSE = CONTENT->PARSE;
    TOKEN_t * TOKEN = (TOKEN_t *)PARSE;
    LIST_t * LIST = (LIST_t *)PARSE;
    INBUF_t * INBUF = (INBUF_t *)PARSE;
    unsigned char nprop;
    char so;        // offset to next state, signed
    state_t ti, ni;
    success_t rc;
    elem_t *branch = new_list(LIST, si);
    static unsigned char nullstring[] = { '\0' };

//E();

    rc = SUCCESS;
    emit_start_state(CONTENT, si, prop, nest, repc);

    nest++;
    assert(nest >= 0);            // catch overflows

    if (!INBUF->inbuf) {          // state_machine just started
        TOKEN->bi = WS;           // pretend preceeded by WS to satisfy toplevel SREP or REP
                                  // (Note, first REP of a sequence *can* be preceeded
                                  // by WS, just not the rest of the REPs. )
        TOKEN->in = nullstring;   // fake it;
        TOKEN->insi = NLL;        // pretend last input was the EOF of a prior file.
    }

    // Entering state
    TOKEN->state = si;            // record of last state entered, for error messages.

    // deal with "terminal" states: Whitespace, Tokens, and Contained activity, Strings

    TOKEN->ei = TOKEN->insi;      // the char class that ended the last token

    // Whitespace
    if ((rc = token_whitespace(TOKEN)) == FAIL) {
        goto done;                // EOF during whitespace
    }

    // Special character tokens
    if (si == TOKEN->insi) {      // single character terminals matching
                                  //  state_machine expectation
        TOKEN->bi = TOKEN->insi;
        rc = token(TOKEN);
        TOKEN->ei = TOKEN->insi;
        goto done;
    }

    switch (si) {
    case ACTIVITY:                // Recursion into Contained activity
        if (TOKEN->bi == LBE) {   // if not top-level of containment
            TOKEN->bi = NLL;
            // recursively process contained ACTIVITY in to its own root
            rc = parse_content_r(PARSE, CONTENT->subject);
            TOKEN->bi = TOKEN->insi; // The char class that terminates the ACTIVITY
            goto done;
        }
        break;

    case STRING:                  // Strings
        rc = token_string(TOKEN, branch);
        TOKEN->bi = TOKEN->insi;  // the char class that terminates the STRING
        goto done;
        break;

    case VSTRING:                 // Value Strings
        rc = token_vstring(TOKEN, branch);
        TOKEN->bi = TOKEN->insi;  // the char class that terminates the VSTRING
        goto done;
        break;

    // the remainder of the switch() is just state initialization
    case SUBJECT:
        TOKEN->has_ast = 0;       // maintain flag for '*' found anywhere in the subject
        PARSE->has_cousin = 0;    // maintain flag for any NODEREF to COUSIN
                                  //  (requiring involvement of ancestors)
        break;
    case COUSIN:
        PARSE->has_cousin = 1;    // maintain a flag for any NODEREF to COUSIN
                                  //  (requiring involvement of ancestors)
        break;
    default:
        break;
    }

    // If it wasn't a terminal state, then use the state_machine to
    // iterate through ALTs or sequences, and then recursively process next the state

    rc = FAIL;                          // init rc to FAIL in case no ALT is satisfied
    ti = si;
    while ((so = state_machine[ti])) {  // iterate over ALTs or sequences
        nprop = state_props[ti];        // get the props for the transition
                                        // from the current state (OPT, ALT, REP etc)

                                        // at this point, ni is a signed, non-zero
                                        // offset to the next state
        ni = ti + so;                   // we get to the next state by adding the
                                        // offset from the current state.

        if (nprop & ALT) {              // look for ALT
            if ((rc = parse_list_r(CONTENT, branch, ni, nprop, nest, 0)) == SUCCESS) {
                break;                  // ALT satisfied
            }
            // we failed an ALT so continue iteration to try next ALT
        } else {                        // else it is a sequence (or the last ALT, same thing)
            repc = 0;
            if (nprop & OPT) {          // OPTional
                if ((rc = parse_list_r(CONTENT, branch, ni, nprop, nest, repc++)) == SUCCESS) {
                    while (parse_more_rep(PARSE, nprop) == SUCCESS) {
                        if ((rc = parse_list_r(CONTENT, branch, ni, nprop, nest, repc++)) != SUCCESS) {
                            break;
                        }
                    }
                }
                rc = SUCCESS;           // OPTs always successful
            } else {                    // else not OPTional, at least one is mandatory
                if ((rc = parse_list_r(CONTENT, branch, ni, nprop, nest, repc++)) != SUCCESS) {
                    break;
                }
                while (parse_more_rep(PARSE, nprop) == SUCCESS) {
                    if ((rc = parse_list_r(CONTENT, branch, ni, nprop, nest, repc++)) != SUCCESS) {
                        break;
                    }
                }
                rc = SUCCESS;           // OPTs always successful
            }
        }
        ti++;        // next ALT (if not yet satisfied), or next sequence item
    }

done: // State exit processing
    if (rc == SUCCESS) {
        switch (si) {
        case ACT:
            rc = doact(CONTENT, branch);              // ACT is complete, process it
            break;
        case LBR:
        case RBR:
        case LAN:
        case RAN:
        case LPN:
        case RPN:
        case TIC:
            // Ignore terminals that are no longer usefusl
            break;
        case EQL:
            // we can ignore EQL in VALASSIGN, but not in samas locations
            if (root->state != (char)VALASSIGN) {
                append_addref(root, branch);  // still needed for sameas
            }
            break;
        default:
            append_addref(root, branch);
            break;
        }
    } 
    free_list(LIST, branch);
    nest--;
    assert(nest >= 0);

    emit_end_state(CONTENT, si, rc, nest, repc);

//E();
    return rc;
}

/**
 * test for more repetitions, emit a separator only if mandated
 *
 * @param PARSE context
 * @param prop properties from grammar
 * @return success = more, fail - no more
 */
static success_t parse_more_rep(PARSE_t * PARSE, unsigned char prop)
{
    TOKEN_t * TOKEN = (TOKEN_t *)PARSE;
    state_t ei, bi;

    if (!(prop & (REP | SREP)))
        return FAIL;

    ei = TOKEN->ei;
    if (ei == RPN || ei == RAN || ei == RBR || ei == RBE) {
        return FAIL;       // no more repetitions
    }
    bi = TOKEN->bi;
    if (bi == RPN || bi == RAN || bi == RBR || bi == RBE
        || (ei != ABC && ei != AST && ei != DQT)) {
        return SUCCESS;    // more repetitions, but additional WS sep is optional
    }
    if (prop & SREP) {
        emit_sep(PARSE);   // sep is non-optional, emit the minimal sep
                           //    .. when low-level emit hook is used.
    }
    return SUCCESS;        // more repetitions
}
