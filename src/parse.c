/* vim:set shiftwidth=4 ts=8 expandtab: */

#if 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "libje_private.h"

#else

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "inbuf.h"
#include "grammar.h"
#include "list.h"
#include "token.h"
#include "ikea.h"

#endif

// forward declarations

static success_t parse_r(container_CONTEXT_t * CC, elem_t * root,
    state_t si, unsigned char prop, int nest, int repc);

static success_t parse_more_rep(CONTEXT_t * C, unsigned char prop);

/**
 * parse G syntax input
 *
 * This parser recurses at two levels:
 *
 *    main() --> je_parse(C) --> parse_r(CC) -| -|  
 *                 ^               ^          |  |
 *                 |               |          |  |
 *                 |               ------<-----  |
 *                 |                             |
 *                 -------------<-----------------
 *
 * The inner recursions are through the grammar state_machine at a single
 * level of containment - maintained in container_context (CC)
 *
 * The outer recursions are through nested containment.
 * The top-level context (C) is available to both and maintains the input state.
 *
 * @param C context
 * @param name
 * @return success/fail
 */

success_t je_parse(CONTEXT_t * C, elem_t * name)
{
    container_CONTEXT_t container_context = { 0 };
    container_CONTEXT_t *CC = &container_context;
    elem_t root = { 0 };    // the output parse tree
    success_t rc;
#if 0
    hash_elem_t *hash_elem;
    uint64_t hash;
    char hashname[12], *filename;
#endif
    TOKEN_t * TOKEN = (TOKEN_t *)C;
    LIST_t * LIST = (LIST_t *)C;

    CC->context = C;
    CC->ikea_box = ikea_box_open(C->ikea_store, NULL);
#if 0
// old, to be removed
    je_hash_list(&hash, name); // hash name (subject "names" can be very long)
    hash_elem = je_hash_bucket(C, hash);    // save in bucket list 
    if (! hash_elem->out) {          // open file, if not already open
        je_long_to_base64(hashname, &hash);
        if (! (filename = malloc(strlen(C->tempdir) + 1 + strlen(hashname) + 1)))
            fatal_perror("Error - malloc(): ");
        strcpy(filename, C->tempdir);
        strcat(filename, "/");
        strcat(filename, hashname);
        hash_elem->out = fopen(filename,"a+b"); //open for binary append writes, + read.
        if (! hash_elem->out)
            fatal_perror("Error - fopen(): ");
        free(filename);
    }
    CC->out = hash_elem->out;
//==============================================================
#endif
CC->out = stdout;

    emit_start_activity(CC);
    C->containment++;            // containment nesting level
    C->stat_containercount++;    // number of containers
    if ((rc = parse_r(CC, &root, ACTIVITY, SREP, 0, 0)) != SUCCESS) {
        if (TOKEN->insi == NLL) {    // EOF is OK
            rc = SUCCESS;
        } else {
            je_token_error(TOKEN, TOKEN->state, "Parse error. Last good state was:");
        }
    }
    C->containment--;
    emit_end_activity(CC);

    ikea_box_close ( CC->ikea_box );

    free_list(LIST, &root);
    free_list(LIST, &(CC->subject));
    free_list(LIST, &(CC->node_pattern_acts));
    free_list(LIST, &(CC->edge_pattern_acts));

    return rc;
}

/** 
 * recurse through state-machine at a single level of containment
 *
 *  @param CC container context
 *  @param root of parsed tree
 *  @return success/fail
 */
static success_t
parse_r(container_CONTEXT_t * CC, elem_t * root,
    state_t si, unsigned char prop, int nest, int repc)
{
    CONTEXT_t *C = CC->context;
    TOKEN_t * TOKEN = (TOKEN_t *)C;
    LIST_t * LIST = (LIST_t *)C;
    INBUF_t * INBUF = (INBUF_t *)C;
    unsigned char nprop;
    char so;        // offset to next state, signed
    state_t ti, ni;
    success_t rc;
    elem_t branch = { 0 };
    elem_t *elem;
    uint64_t hash;
    static unsigned char nullstring[] = { '\0' };

    rc = SUCCESS;
    emit_start_state(CC, si, prop, nest, repc);
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
            rc = je_parse(C, &CC->subject);    // recursively process contained ACTIVITY in to its own root
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
        C->has_cousin = 0;  // maintain a flag for any NODEREF to COUSIN (requiring involvement of ancestors)
        break;
    case COUSIN:
        C->has_cousin = 0;  // maintain a flag for any NODEREF to COUSIN (requiring involvement of ancestors)
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
            if ((rc = parse_r(CC, &branch, ni, nprop, nest, 0)) == SUCCESS) {
                break;                  // ALT satisfied
            }

                                        // we failed an ALT so continue iteration to try next ALT
        } else {                        // else it is a sequence (or the last ALT, same thing)
            repc = 0;
            if (nprop & OPT) {          // OPTional
                if ((parse_r(CC, &branch, ni, nprop, nest, repc++)) == SUCCESS) {
                    while (parse_more_rep(C, nprop) == SUCCESS) {
                        if (parse_r(CC, &branch, ni, nprop, nest, repc++) == FAIL) {
                            break;
                        }
                    }
                }
            } else {                    // else not OPTional
                if ((rc = parse_r(CC, &branch, ni, nprop, nest, repc++)) == FAIL) {
                    break;
                }
                while (parse_more_rep(C, nprop) == SUCCESS) {
                    if ((rc = parse_r(CC, &branch, ni, nprop, nest, repc++)) == FAIL) {
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
            C->stat_inactcount++;
            if (CC->is_pattern) {   // flag was set by SUBJECT in previous ACT
                                    //  save entire previous ACT in a list of pattern_acts
                C->stat_patternactcount++;
                if (CC->subject_type == NODE) {
                    append_list(&(CC->node_pattern_acts), move_list(LIST, &branch));

                } else {
                    assert(CC->subject_type == EDGE);
                    append_list(&(CC->edge_pattern_acts), move_list(LIST, &branch));
                }
            } else {
                C->stat_nonpatternactcount++;
                append_list(root, move_list(LIST, &branch));

                // dispatch events for the ACT just finished
                je_dispatch(CC, root);

// and this is where we actually emit the fully processed acts!
//  (there can be multiple acts after pattern subst.  Each matched pattern generates an additional act.
                elem = root->first;
                while (elem) {
                    C->stat_outactcount++;
                    emit_act(CC, elem);  // emit hook for rewritten act
                    je_emit_ikea(CC, elem);  // primary emitter to ikea store
//                    je_emit_act(CC, elem);  // primary emitter to graph DB
//fprintf(stdout,"####\n");
                    je_emit_act2(CC, elem);  // primary emitter to graph DB

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
            ikea_store_snapshot(C->ikea_store);
            // FIXME - open appending container for this box.
            break;
        case SUBJECT: // subject rewrites before adding branch to root
            branch.state = si;

            // Perform EQL "same as in subject of previous ACT" substitutions
            // Also classifies ACT as NODE or EDGE based on SUBJECT
P(&branch);
            je_sameas(CC, &branch);

            je_hash_list(&hash, &(CC->subject));   // generate name hash
            (void)je_hash_bucket(C, hash);    // save in bucket list 

            // If this subject is not itself a pattern, then
            // perform pattern matching and insertion if matched
            if (!(CC->is_pattern = TOKEN->has_ast)) {
                je_pattern(CC, root, &branch);
            }

            emit_subject(CC, &branch);      // emit hook for rewritten subject
            break;
        case ATTRIBUTES:
            emit_attributes(CC, &branch);   // emit hook for attributes
            break;
        default:
            break;
        }
        if (branch.first != NULL || si == EQL) {    // mostly ignore empty lists
            if (branch.first && branch.first->type != FRAGELEM) {
                // record state generating this tree
                // - except for STRINGs which use state for quoting info
                branch.state = si;
            }
            append_list(root, move_list(LIST, &branch));
        }
    }
    nest--;
    assert(nest >= 0);
    emit_end_state(CC, si, rc, nest, repc);

    return rc;
}

/**
 * test for more repetitions, emit a separator only if mandated
 *
 * @param C context
 * @param prop properties from grammar
 * @return success = more, fail - no more
 */
static success_t parse_more_rep(CONTEXT_t * C, unsigned char prop)
{
    TOKEN_t * TOKEN = (TOKEN_t *)C;
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
        emit_sep(C);    // sep is non-optional, emit the minimal sep
                        //    .. when low-level emit hook is used.
    }
    return SUCCESS;        // more repetitions
}
