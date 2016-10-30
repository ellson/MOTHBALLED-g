/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "parse.h"

// forward declarations

static success_t
parse_nest_r(PARSE_t * PARSE, elem_t * subject);

static elem_t *
parse_list_r(CONTENT_t * CONTENT, state_t si, unsigned char prop, int nest, int repc);

static success_t
parse_more_rep(PARSE_t * PARSE, unsigned char prop);

/**
 * parse G syntax input
 *
 * This parser recurses at two levels:
 *
 * parse() --> parse_nest_r() --> parse_list_r() -| -|  
 *           ^                  ^                 |  |
 *           |                  |                 |  |
 *           |                  ---------<--------|  |
 *           |                                       |
 *           --------------------<-------------------|
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

//E(PARSE);
    rc = parse_nest_r(PARSE, NULL);
//E(PARSE);
    return rc;
}

static success_t parse_nest_r(PARSE_t * PARSE, elem_t * subject)
{
    CONTENT_t container_context = { 0 };
    CONTENT_t * CONTENT = &container_context;
    success_t rc;
    TOKEN_t * TOKEN = (TOKEN_t *)PARSE;
    LIST_t * LIST = (LIST_t *)PARSE;
    elem_t *root;

E(LIST);

    CONTENT->PARSE = PARSE;
    CONTENT->subject = new_list(LIST, SUBJECT);
    CONTENT->node_pattern_acts = new_list(LIST, 0);
    CONTENT->edge_pattern_acts = new_list(LIST, 0);
    CONTENT->ikea_box = ikea_box_open(PARSE->ikea_store, NULL);
    CONTENT->out = stdout;
    emit_start_activity(CONTENT);
    PARSE->containment++;            // containment nesting level
    PARSE->stat_containercount++;    // number of containers

    root = parse_list_r(CONTENT, ACTIVITY, SREP, 0, 0);
    if (root) {
        if (TOKEN->insi == NLL) {    // EOF is OK
            rc = SUCCESS;
        } else {
            token_error(TOKEN, TOKEN->state, "Parse error. Last good state was:");
        }
    }
//P(LIST,root);
E(LIST);

    if (CONTENT->nodes) {
        PARSE->sep = '\0';
        print_tree(LIST, CONTENT->nodes);
        putc('\n', stdout);
        free_tree(LIST, CONTENT->nodes);
    }
    if (CONTENT->edges) {
        PARSE->sep = '\0';
        print_tree(LIST, CONTENT->edges);
        putc('\n', stdout);
        free_tree(LIST, CONTENT->edges);
    }
    PARSE->containment--;
    emit_end_activity(CONTENT);

    ikea_box_close ( CONTENT->ikea_box );

E(LIST);
    free_list(LIST, root);
    free_list(LIST, CONTENT->subject);
    free_list(LIST, CONTENT->node_pattern_acts);
    free_list(LIST, CONTENT->edge_pattern_acts);

E(LIST);
    return rc;
}

/** 
 * recurse through state-machine at a single level of containment
 *
 *  @param CONTENT container context
 *  @param si input state
 *  @param prop grammar properties
 *  @param nest recursion nesting level (containment level)
 *  @param repc sequence member counter
 *  @return resulting parsed branch
 */
static elem_t *
parse_list_r(CONTENT_t * CONTENT, state_t si, unsigned char prop, int nest, int repc)
{
    PARSE_t * PARSE = CONTENT->PARSE;
    TOKEN_t * TOKEN = (TOKEN_t *)PARSE;
    LIST_t * LIST = (LIST_t *)PARSE;
    INBUF_t * INBUF = (INBUF_t *)PARSE;
    unsigned char nprop;
    char so;        // offset to next state, signed
    state_t ti, ni;
    success_t rc;
    elem_t *branch, *elem, *new;
    static unsigned char nullstring[] = { '\0' };

E(LIST);

    branch = new_list(LIST, si);  // return list

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
            rc = parse_nest_r(PARSE, CONTENT->subject);
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
    case ACT:
        TOKEN->verb = 0;          // initialize verb to default "add"
        break;
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
            new = parse_list_r(CONTENT, ni, nprop, nest, 0);
            if (new) {
                append_list_move(branch, new);
                rc = SUCCESS;
                break;                  // ALT satisfied
            }
            // we failed an ALT so continue iteration to try next ALT
        } else {                        // else it is a sequence (or the last ALT, same thing)
            repc = 0;
            if (nprop & OPT) {          // OPTional
                new = parse_list_r(CONTENT, ni, nprop, nest, repc++);
                if (new) {
                    append_list_move(branch, new);
                    while (parse_more_rep(PARSE, nprop) == SUCCESS) {
                        new = parse_list_r(CONTENT, ni, nprop, nest, repc++);
                        if (new) {
                            append_list_move(branch, new);
                        } else {
                            break;
                        }
                    }
                }
                rc = SUCCESS;           // OPTs always successful
            } else {                    // else not OPTional
                new = parse_list_r(CONTENT, ni, nprop, nest, repc++);
                if (new) {
                    append_list_move(branch, new);
                    rc = SUCCESS;           // OPTs always successful
                } else {
                    break;
                }
                while (parse_more_rep(PARSE, nprop) == SUCCESS) {
                    new = parse_list_r(CONTENT, ni, nprop, nest, repc++);
                    if (new) {
                        append_list_move(branch, new);
                        rc = SUCCESS;           // OPTs always successful
                    } else {
                        break;
                    }
                }
            }
        }
        ti++;        // next ALT (if not yet satisfied), or next sequence item
    }

done: // State exit processing
//P(LIST, branch);
    if (rc == SUCCESS) {
        switch (si) {
        case ACT:
            PARSE->stat_inactcount++;
            if (CONTENT->is_pattern) {   // flag was set by SUBJECT in previous ACT
                                         //  save entire previous ACT in a list of pattern_acts
                PARSE->stat_patternactcount++;
                assert(CONTENT->subject_type == NODE || CONTENT->subject_type == EDGE);
                if (CONTENT->subject_type == NODE) {
                    append_list_move(CONTENT->node_pattern_acts, branch);
                } else {
                    append_list_move(CONTENT->edge_pattern_acts, branch);
                }
                branch = NULL;
            } else {
                PARSE->stat_nonpatternactcount++;

P(LIST, branch);
                // dispatch events for the ACT just finished
                new = dispatch(CONTENT, branch);
                free_list(LIST, branch);
                branch = new;
P(LIST, branch);

// and this is where we actually emit the fully processed acts!
//  (there can be multiple acts after pattern subst.  Each matched pattern generates an additional act.
                elem = branch->u.l.first;
                while (elem) {
                    PARSE->stat_outactcount++;
P(LIST,elem);
//                    je_emit_act(CONTENT, elem);  // primary emitter to graph DB
                    reduce(CONTENT, elem);  // eliminate reduncy by insertion sorting into trees.

                    elem = elem->u.l.next;
                }
                // FIXME - should the branch be freed here?
            }
            break;
        case TLD:
        case QRY:
            TOKEN->verb = si;  // record verb prefix, if not default
            break;
        case HAT:
            // FIXME - this  is all wrong ... maybe it should just close the current box
            // FIXME - close this container's box
            ikea_store_snapshot(PARSE->ikea_store);
            // FIXME - open appending container for this box.
            break;
        case SUBJECT: // subject rewrites before adding branch to root
            branch->state = si;

            // Perform EQL "same as in subject of previous ACT" substitutions
            // Also classifies ACT as NODE or EDGE based on SUBJECT
//P(LIST, branch);
            new = sameas(CONTENT, branch);
            free_list(LIST, branch);
            branch = new;
//P(LIST, branch);

            // If this subject is not itself a pattern, then
            // perform pattern matching and insertion if matched
            if (!(CONTENT->is_pattern = TOKEN->has_ast)) {
                new = pattern(CONTENT, branch);
                if (new) {
                    free_list(LIST, branch);
                    branch = new;
                }
            }

            emit_subject(CONTENT, branch);      // emit hook for rewritten subject
            break;
        case ATTRIBUTES:
            emit_attributes(CONTENT, branch);   // emit hook for attributes
            break;
        default:
            break;
        }
        if (branch->u.l.first != NULL || si == EQL) {    // mostly ignore empty lists
            if (branch->u.l.first && branch->u.l.first->type != FRAGELEM) {
                // record state generating this tree
                // - except for STRINGs which use state for quoting info
                branch->state = si;
            }
        }
    }
    nest--;
    assert(nest >= 0);

    emit_end_state(CONTENT, si, rc, nest, repc);

    if (rc == FAIL) {
        return NULL;
    }

E(LIST);
    return branch;
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
