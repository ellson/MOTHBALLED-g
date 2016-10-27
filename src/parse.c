/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "parse.h"

// forward declarations

static success_t parse_nest_r(PARSE_t * PARSE, elem_t * name);

static success_t parse_r(CONTENT_t * CONTENT, elem_t * root,
    state_t si, unsigned char prop, int nest, int repc);

static success_t parse_more_rep(PARSE_t * PARSE, unsigned char prop);

/**
 * parse G syntax input
 *
 * This parser recurses at two levels:
 *
 *    je_parse() --> parse_nest_r(PARSE) --> parse_r(CONTENT) -| -|  
 *                 ^                   ^              |  |
 *                 |                   |              |  |
 *                 |                   -------<-------|  |
 *                 |                                     |
 *                 -------------------<------------------|
 *
 * The outer recursions are through nested containment.
 *
 * The inner recursions are through the grammar state_machine at a single
 * level of containment - maintained in container_context (CONTENT)
 *
 * The top-level context (PARSE) is available to both and maintains the input state.
 *
 * @param PARSE context
 * @param name
 * @return success/fail
 */

success_t je_parse(PARSE_t * PARSE)
{
    return parse_nest_r(PARSE, NULL);
}

static success_t parse_nest_r(PARSE_t * PARSE, elem_t * name)
{
    CONTENT_t container_context = { 0 };
    CONTENT_t * CONTENT = &container_context;
    elem_t root = { 0 };    // the output parse tree
    success_t rc;
#if 0
    hash_elem_t *hash_elem;
    uint64_t hash;
    char hashname[12], *filename;
#endif
    TOKEN_t * TOKEN = (TOKEN_t *)PARSE;
    LIST_t * LIST = (LIST_t *)PARSE;

    CONTENT->PARSE = PARSE;
    CONTENT->ikea_box = ikea_box_open(PARSE->ikea_store, NULL);
#if 0
// old, to be removed
    je_hash_list(&hash, name); // hash name (subject "names" can be very long)
    hash_elem = je_hash_bucket(PARSE, hash);    // save in bucket list 
    if (! hash_elem->out) {          // open file, if not already open
        je_long_to_base64(hashname, &hash);
        if (! (filename = malloc(strlen(PARSE->tempdir) + 1 + strlen(hashname) + 1)))
            fatal_perror("Error - malloc(): ");
        strcpy(filename, PARSE->tempdir);
        strcat(filename, "/");
        strcat(filename, hashname);
        hash_elem->out = fopen(filename,"a+b"); //open for binary append writes, + read.
        if (! hash_elem->out)
            fatal_perror("Error - fopen(): ");
        free(filename);
    }
    CONTENT->out = hash_elem->out;
//==============================================================
#endif
CONTENT->out = stdout;

    emit_start_activity(CONTENT);
    PARSE->containment++;            // containment nesting level
    PARSE->stat_containercount++;    // number of containers
    if ((rc = parse_r(CONTENT, &root, ACTIVITY, SREP, 0, 0)) != SUCCESS) {
        if (TOKEN->insi == NLL) {    // EOF is OK
            rc = SUCCESS;
        } else {
            je_token_error(TOKEN, TOKEN->state, "Parse error. Last good state was:");
        }
    }
    if (CONTENT->nodes) {
        PARSE->sep = '\0';
        print_tree(PARSE, CONTENT->nodes);
        putc('\n', stdout);
    }
    if (CONTENT->edges) {
        PARSE->sep = '\0';
        print_tree(PARSE, CONTENT->edges);
        putc('\n', stdout);
    }
    PARSE->containment--;
    emit_end_activity(CONTENT);

    ikea_box_close ( CONTENT->ikea_box );

    free_list(LIST, &root);
    free_list(LIST, &(CONTENT->subject));
    free_list(LIST, &(CONTENT->node_pattern_acts));
    free_list(LIST, &(CONTENT->edge_pattern_acts));

    return rc;
}

/** 
 * recurse through state-machine at a single level of containment
 *
 *  @param CONTENT container context
 *  @param root of parsed tree
 *  @return success/fail
 */
static success_t
parse_r(CONTENT_t * CONTENT, elem_t * root,
    state_t si, unsigned char prop, int nest, int repc)
{
    PARSE_t * PARSE = CONTENT->PARSE;
    TOKEN_t * TOKEN = (TOKEN_t *)PARSE;
    LIST_t * LIST = (LIST_t *)PARSE;
    INBUF_t * INBUF = (INBUF_t *)PARSE;
    unsigned char nprop;
    char so;        // offset to next state, signed
    state_t ti, ni;
    success_t rc;
    elem_t branch = { 0 };
    elem_t *elem;
    static unsigned char nullstring[] = { '\0' };

    rc = SUCCESS;
    emit_start_state(CONTENT, si, prop, nest, repc);
    branch.state = si;

    nest++;
    assert(nest >= 0);    // catch overflows

    if (!INBUF->inbuf) {    // state_machine just started
        TOKEN->bi = WS;        // pretend preceeded by WS to satisfy toplevel SREP or REP
                        // (Note, first REP of a sequence *can*
                        // be preceeded by WS, just not the
                        // rest of the REPs. )
        TOKEN->in = nullstring;    // fake it;
        TOKEN->insi = NLL;    // pretend last input was the EOF of
                        // a prior file.
    }

    // Entering state
    TOKEN->state = si;        // record of last state entered, for error messages.

    // deal with "terminal" states: Whitespace, Tokens, and Contained activity, Strings

    TOKEN->ei = TOKEN->insi;    // the char class that ended the last token

    // Whitespace
    if ((rc = je_token_whitespace(TOKEN)) == FAIL) {
        goto done;    // EOF during whitespace
    }

    // Special character tokens
    if (si == TOKEN->insi) {    // single character terminals matching state_machine expectation
        TOKEN->bi = TOKEN->insi;
        rc = je_token(TOKEN);
        TOKEN->ei = TOKEN->insi;
        goto done;
    }
    switch (si) {
    case ACTIVITY:          // Recursion into Contained activity
        if (TOKEN->bi == LBE) {    // if not top-level of containment
            TOKEN->bi = NLL;
            rc = parse_nest_r(PARSE, &CONTENT->subject);    // recursively process contained ACTIVITY in to its own root
            TOKEN->bi = TOKEN->insi;    // The char class that terminates the ACTIVITY
            goto done;
        }
        break;

    case STRING:            // Strings
        rc = je_token_string(TOKEN, &branch);
        TOKEN->bi = TOKEN->insi;    // the char class that terminates the STRING
        goto done;
        break;

    case VSTRING:            // Value Strings
        rc = je_token_vstring(TOKEN, &branch);
        TOKEN->bi = TOKEN->insi;    // the char class that terminates the VSTRING
        goto done;
        break;

    // the remainder of the switch() is just state initialization
    case ACT:
        TOKEN->verb = 0;        // initialize verb to default "add"
        break;
    case SUBJECT:
        TOKEN->has_ast = 0;     // maintain a flag for an '*' found anywhere in the subject
        PARSE->has_cousin = 0;  // maintain a flag for any NODEREF to COUSIN (requiring involvement of ancestors)
        break;
    case COUSIN:
        PARSE->has_cousin = 0;  // maintain a flag for any NODEREF to COUSIN (requiring involvement of ancestors)
        break;
    default:
        break;
    }

    // If it wasn't a terminal state, then use the state_machine to
    // iterate through ALTs or sequences, and then recursively process next the state

    rc = FAIL;        // init rc to FAIL in case no ALT is satisfied
    ti = si;
    while ((so = state_machine[ti])) {    // iterate over ALTs or sequences
        nprop = state_props[ti];        // get the props for the transition
                                        // from the current state (OPT, ALT, REP etc)

                                        // at this point, ni is a signed, non-zero
                                        // offset to the next state
        ni = ti + so;                   // we get to the next state by adding the
                                        // offset from the current state.

        if (nprop & ALT) {              // look for ALT
            if ((rc = parse_r(CONTENT, &branch, ni, nprop, nest, 0)) == SUCCESS) {
                break;                  // ALT satisfied
            }

                                        // we failed an ALT so continue iteration to try next ALT
        } else {                        // else it is a sequence (or the last ALT, same thing)
            repc = 0;
            if (nprop & OPT) {          // OPTional
                if ((parse_r(CONTENT, &branch, ni, nprop, nest, repc++)) == SUCCESS) {
                    while (parse_more_rep(PARSE, nprop) == SUCCESS) {
                        if (parse_r(CONTENT, &branch, ni, nprop, nest, repc++) == FAIL) {
                            break;
                        }
                    }
                }
            } else {                    // else not OPTional
                if ((rc = parse_r(CONTENT, &branch, ni, nprop, nest, repc++)) == FAIL) {
                    break;
                }
                while (parse_more_rep(PARSE, nprop) == SUCCESS) {
                    if ((rc = parse_r(CONTENT, &branch, ni, nprop, nest, repc++)) == FAIL) {
                        break;
                    }
                }
            }
        }
        ti++;        // next ALT (if not yet satisfied), or next sequence item
    }

 done: // State exit processing
    if (rc == SUCCESS) {
        switch (si) {
        case ACT:
            PARSE->stat_inactcount++;
            if (CONTENT->is_pattern) {   // flag was set by SUBJECT in previous ACT
                                    //  save entire previous ACT in a list of pattern_acts
                PARSE->stat_patternactcount++;
                if (CONTENT->subject_type == NODE) {
                    append_list(&(CONTENT->node_pattern_acts), move_list(LIST, &branch));

                } else {
                    assert(CONTENT->subject_type == EDGE);
                    append_list(&(CONTENT->edge_pattern_acts), move_list(LIST, &branch));
                }
            } else {
                PARSE->stat_nonpatternactcount++;
                append_list(root, move_list(LIST, &branch));

                // dispatch events for the ACT just finished
                je_dispatch(CONTENT, root);

// and this is where we actually emit the fully processed acts!
//  (there can be multiple acts after pattern subst.  Each matched pattern generates an additional act.
                elem = root->u.l.first;
                while (elem) {
                    PARSE->stat_outactcount++;
//                    je_emit_act(CONTENT, elem);  // primary emitter to graph DB
                    je_reduce(CONTENT, elem);  // eliminate reduncy by insertion sorting into trees.

                    elem = elem->next;
                }

                free_list(LIST, root);  // that's all folks.  move on to the next ACT.
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
            branch.state = si;

            // Perform EQL "same as in subject of previous ACT" substitutions
            // Also classifies ACT as NODE or EDGE based on SUBJECT
            je_sameas(CONTENT, &branch);

// FIXME - or not, but this is broken
#if 0
            je_hash_list(&hash, &(CONTENT->subject));   // generate name hash
            (void)je_hash_bucket(PARSE, hash);    // save in bucket list 
#endif

            // If this subject is not itself a pattern, then
            // perform pattern matching and insertion if matched
            if (!(CONTENT->is_pattern = TOKEN->has_ast)) {
                je_pattern(CONTENT, root, &branch);
            }

            emit_subject(CONTENT, &branch);      // emit hook for rewritten subject
            break;
        case ATTRIBUTES:
            emit_attributes(CONTENT, &branch);   // emit hook for attributes
            break;
        default:
            break;
        }
        if (branch.u.l.first != NULL || si == EQL) {    // mostly ignore empty lists
            if (branch.u.l.first && branch.u.l.first->type != FRAGELEM) {
                // record state generating this tree
                // - except for STRINGs which use state for quoting info
                branch.state = si;
            }
            append_list(root, move_list(LIST, &branch));
        }
    }
    nest--;
    assert(nest >= 0);
    emit_end_state(CONTENT, si, rc, nest, repc);

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
        return FAIL;    // no more repetitions
    }
    bi = TOKEN->bi;
    if (bi == RPN || bi == RAN || bi == RBR || bi == RBE
        || (ei != ABC && ei != AST && ei != DQT)) {
        return SUCCESS;    // more repetitions, but additional WS sep is optional
    }
    if (prop & SREP) {
        emit_sep(PARSE);    // sep is non-optional, emit the minimal sep
                        //    .. when low-level emit hook is used.
    }
    return SUCCESS;        // more repetitions
}
