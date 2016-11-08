/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "types.h"
#include "inbuf.h"
#include "list.h"
#include "grammar.h"
#include "token.h"
#include "graph.h"
#include "container.h"
#include "doact.h"

#define TOKEN() ((TOKEN_t*)THREAD)
#define LIST() ((LIST_t*)THREAD)
#define INBUF() ((INBUF_t*)THREAD)

// FIXME - doxygen doesn't like this ASCII diagram
//            -- special  mean of trailing | ???

/**
 * parse G syntax input
 *
 * This parser recurses at two levels:
 *
 *  input
 *    |
 *    V
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
 * The inner recursions are through the grammar state_machine
 * at a single level of containment.
 * 
 * The top-level THREAD context is available to both and
 * maintains the input state.
 */

#define MORE_REP(prop, ei) \
    ((prop & (REP | SREP)) && ei != RPN && ei != RAN && ei != RBR && ei != RBE)

/**
 * iterate and recurse through state-machine at a single level of containment
 *
 *  @param GRAPH context
 *  @param root - the parent's branch that we are adding to
 *  @param si input state
 *  @param prop grammar properties
 *  @param nest recursion nesting level (containment level)
 *  @param repc sequence member counter
 *  @param bi from state
 *  @return SUCCESS or FAIL
 */
success_t graph(GRAPH_t * GRAPH, elem_t *root, state_t si, unsigned char prop, int nest, int repc, state_t bi)
{
    THREAD_t * THREAD = ((CONTAINER_t*)GRAPH)->THREAD;
    unsigned char nprop;
    char so;        // offset to next state, signed
    state_t ti, ni, ei;
    success_t rc;
    elem_t *branch;
    static unsigned char nullstring[] = { '\0' };

//E();

    rc = SUCCESS;
    branch = new_list(LIST(), si);
    nest++;
    assert(nest >= 0);            // catch overflows

    if (! INBUF()->inbuf) {       // state_machine just started
        bi = WS;                  // pretend preceeded by WS to satisfy toplevel SREP or REP
                                  // (Note, first REP of a sequence *can* be preceeded
                                  // by WS, just not the rest of the REPs. )
        TOKEN()->in = nullstring; // fake it;
        TOKEN()->insi = NLL;      // pretend last input was the EOF of a prior file.
    }

    // Entering state
    TOKEN()->state = bi;          // record of last state entered, for error messages.

    // deal with "terminal" states: Whitespace, Tokens, and Contained activity, Strings

    ei = TOKEN()->insi;  // the char class that ended the last token

    // Whitespace
    if ((rc = token_whitespace(TOKEN())) == FAIL) {
        goto done;                // EOF during whitespace
    }

    // Special character tokens
    if (si == TOKEN()->insi) {    // single character terminals matching
                                  //  state_machine expectation
        bi = TOKEN()->insi;
        ei = token(TOKEN());
        goto done;
    }

    switch (si) {
    case ACTIVITY:         // Recursion into Contained activity
        if (bi == LBE) {   // if not top-level of containment
            // recursively process contained ACTIVITY in to its own root
            rc = container(THREAD);
            bi = TOKEN()->insi; // The char class that terminates the ACTIVITY
            goto done;
        }
        break;

    case STRING:                  // Strings
        rc = token_string(TOKEN(), branch);
        bi = TOKEN()->insi;  // the char class that terminates the STRING
        goto done;
        break;

    case VSTRING:                 // Value Strings
        rc = token_vstring(TOKEN(), branch);
        bi = TOKEN()->insi;  // the char class that terminates the VSTRING
        goto done;
        break;

    // the remainder of the switch() is just state initialization
    case ACT:
        GRAPH->verb = 0;          // default "add"
        break;
    case SUBJECT:
        TOKEN()->is_pattern = 0;  // maintain flag for '*' found anywhere in the subject
        GRAPH->need_mum = 0;    // maintain flag for any MUM involvement
        break;
    case MUM:
        GRAPH->need_mum = 1;    // maintain a flag for any MUM involvement
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
            if ((rc = graph(GRAPH, branch, ni, nprop, nest, 0, bi)) == SUCCESS) {
                break;                  // ALT satisfied
            }
            // we failed an ALT so continue iteration to try next ALT
        } else {                        // else it is a sequence (or the last ALT, same thing)
            repc = 0;
            if (nprop & OPT) {          // OPTional
                if ((rc = graph(GRAPH, branch, ni, nprop, nest, repc++, bi)) == SUCCESS) {
                    while (MORE_REP(nprop, ei)) {
                        if ((rc = graph(GRAPH, branch, ni, nprop, nest, repc++, bi)) != SUCCESS) {
                            break;
                        }
                    }
                }
                rc = SUCCESS;           // OPTs always successful
            } else {                    // else not OPTional, at least one is mandatory
                if ((rc = graph(GRAPH, branch, ni, nprop, nest, repc++, bi)) != SUCCESS) {
                    break;
                }
                while (MORE_REP(nprop, ei)) {
                    if ((rc = graph(GRAPH, branch, ni, nprop, nest, repc++, bi)) != SUCCESS) {
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
            rc = doact((CONTAINER_t*)GRAPH, branch);
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
        case FAMILY:
        case RELATIVE:
        case PORT:
        case NODEREF:
        case NOUN:
        case NOUNSET:
        case ENDPOINTSET:
            append_addref(root, branch->u.l.first);
            break;
        case LBR:  // bracketing ATTRs
        case RBR:
        case LAN:  // bracketing LEGs
        case RAN:
        case LPN:  // bracketing NOUNs or ENDPOINTs
        case RPN:
        case TIC:  // prefixing DISAMBID
        case HAT:  // indicating MUM
        case FSL:  // prefixing KID
        case CLN:  // prefixing PORT
            break;
        default:
            // everything else is appended to parent's branch
            append_addref(root, branch);
            break;
        }
    }
    free_list(LIST(), branch);
    nest--;
    assert(nest >= 0);

//E();
    return rc;
}
