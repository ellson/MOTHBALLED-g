/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "thread.h"
#include "print.h"

// forward declarations
static success_t more_rep(TOKEN_t * TOKEN, unsigned char prop);

/**
 * parse G syntax input
 *
 * This parser recurses at two levels:
 *
 * thread() --> container() --> graph() ------| -|  
 *            ^               ^   |           |  |
 *            |               |   -> doact()  |  |
 *            |               |               |  |
 *            |               --------<-------|  |
 *            |                                  |
 *            ----------------<------------------|
 *
 * The outer recursions are through nested containment.
 *
 * The inner recursions are through the grammar state_machine at a single
 * level of containment.
 *
 * The top-level THREAD context is available to both and maintains the input state.
 */

/** 
 * iterate and recurse through state-machine at a single level of containment
 *
 *  @param CONTAINER context
 *  @param root - the parent's branch that we are adding to
 *  @param si input state
 *  @param prop grammar properties
 *  @param nest recursion nesting level (containment level)
 *  @param repc sequence member counter
 *  @return SUCCESS or FAIL
 */
success_t graph(CONTAINER_t * CONTAINER, elem_t *root, state_t si, unsigned char prop, int nest, int repc)
{
    GRAPH_t *GRAPH = (GRAPH_t*)CONTAINER;
    THREAD_t * THREAD = CONTAINER->THREAD;
    TOKEN_t * TOKEN = (TOKEN_t*)THREAD;
    LIST_t * LIST = (LIST_t*)TOKEN;
    INBUF_t * INBUF = (INBUF_t*)LIST;
    unsigned char nprop;
    char so;        // offset to next state, signed
    state_t ti, ni;
    success_t rc;
    elem_t *branch = new_list(LIST, si);
    static unsigned char nullstring[] = { '\0' };

//E();

    rc = SUCCESS;

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
            rc = container(THREAD);
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
        GRAPH->verb = 0;          // default "add"
        break;
    case SUBJECT:
        TOKEN->has_ast = 0;       // maintain flag for '*' found anywhere in the subject
        GRAPH->has_cousin = 0;    // maintain flag for any NODEREF to COUSIN
                                  //  (requiring involvement of ancestors)
        break;
    case COUSIN:
        GRAPH->has_cousin = 1;    // maintain a flag for any NODEREF to COUSIN
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
            if ((rc = graph(CONTAINER, branch, ni, nprop, nest, 0)) == SUCCESS) {
                break;                  // ALT satisfied
            }
            // we failed an ALT so continue iteration to try next ALT
        } else {                        // else it is a sequence (or the last ALT, same thing)
            repc = 0;
            if (nprop & OPT) {          // OPTional
                if ((rc = graph(CONTAINER, branch, ni, nprop, nest, repc++)) == SUCCESS) {
                    while (more_rep(TOKEN, nprop) == SUCCESS) {
                        if ((rc = graph(CONTAINER, branch, ni, nprop, nest, repc++)) != SUCCESS) {
                            break;
                        }
                    }
                }
                rc = SUCCESS;           // OPTs always successful
            } else {                    // else not OPTional, at least one is mandatory
                if ((rc = graph(CONTAINER, branch, ni, nprop, nest, repc++)) != SUCCESS) {
                    break;
                }
                while (more_rep(TOKEN, nprop) == SUCCESS) {
                    if ((rc = graph(CONTAINER, branch, ni, nprop, nest, repc++)) != SUCCESS) {
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

        case ACT:  // ACT is complete, process it
            rc = doact(CONTAINER, branch);
            // this is the top recursion
            // no more need for this branch
            // don't bother appending to root
            break;

        // drop various bits of the tree that are no longer useful
        case VERB:  // VERB - after stashing away its value
            GRAPH->verb = branch->u.l.first->state;  // QRY or TLD
            break;
        case VALASSIGN: // ignore VALASSIGN EQL, but keep VALUE
            append_addref(root, branch->u.l.first->u.l.next);
            break;
        case LBR:  // bracketing ATTRs
        case RBR:
        case LAN:  // bracketing LEGs
        case RAN:
        case LPN:  // bracketing NOUNs or ENDPOINTs
        case RPN:
        case TIC:  // prefixing DISAMBID
        case HAT:  // indicating PARENT
        case FSL:  // prefixing CHILD
        case CLN:  // prefixing PORT
            break;
        default:
            // everything else is appended to parent's branch
            append_addref(root, branch);
            break;
        }
    } 
    free_list(LIST, branch);
    nest--;
    assert(nest >= 0);

//E();
    return rc;
}

/**
 * test for more repetitions
 *
 * @param TOKEN context
 * @param prop properties from grammar
 * @return success = more, fail - no more
 */
static success_t more_rep(TOKEN_t * TOKEN, unsigned char prop)
{
    state_t ei, bi;

    if (!(prop & (REP | SREP))) {
        return FAIL;
    }
    ei = TOKEN->ei;
    if (ei == RPN || ei == RAN || ei == RBR || ei == RBE) {
        return FAIL;       // no more repetitions
    }
    bi = TOKEN->bi;
    if (bi == RPN || bi == RAN || bi == RBR || bi == RBE
        || (ei != ABC && ei != AST && ei != DQT)) {
        return SUCCESS;    // more repetitions, but additional WS sep is optional
    }
    return SUCCESS;        // more repetitions
}
