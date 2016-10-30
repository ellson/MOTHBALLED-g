/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "doact.h"

success_t doact(CONTENT_t *CONTENT, elem_t *act)
{
    PARSE_t * PARSE = CONTENT->PARSE;
    LIST_t *LIST = (LIST_t*)PARSE;

P(LIST, act);

    PARSE->stat_inactcount++;
#if 0
    if (CONTENT->is_pattern) {   // flag was set by SUBJECT in previous ACT
                                         //  save entire previous ACT in a list of pattern_acts
        PARSE->stat_patternactcount++;
        assert(CONTENT->subject_type == NODE || CONTENT->subject_type == EDGE);
        if (CONTENT->subject_type == NODE) {
                    append_transfer(CONTENT->node_pattern_acts, act);
        } else {
                    append_transfer(CONTENT->edge_pattern_acts, act);
        }
    } else {
        PARSE->stat_nonpatternactcount++;

//P(LIST, act);
        // dispatch events for the ACT just finished
        new = dispatch(CONTENT, act);
        if (new) {
            free_list(LIST, act);
            act = new;
        }
//P(LIST, act);

// and this is where we actually emit the fully processed acts!
//  (there can be multiple acts after pattern subst.  Each matched pattern generates an additional act.
        elem = act->u.l.first;
        while (elem) {
            PARSE->stat_outactcount++;
//P(LIST,elem);
//                    je_emit_act(CONTENT, elem);  // primary emitter to graph DB
//                    reduce(CONTENT, elem);  // eliminate reduncy by insertion sorting into trees.

            elem = elem->u.l.next;
        }
    }
#endif
#if 0
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
            act->state = si;

            // Perform EQL "same as in subject of previous ACT" substitutions
            // Also classifies ACT as NODE or EDGE based on SUBJECT
//P(LIST, act);
            new = sameas(CONTENT, act);
            free_list(LIST, act);
            act = new;
//P(LIST, act);

            // If this subject is not itself a pattern, then
            // perform pattern matching and insertion if matched
            if (!(CONTENT->is_pattern = TOKEN->has_ast)) {
                new = pattern(CONTENT, act);
                if (new) {
                    free_list(LIST, act);
                    act = new;
                }
            }

            emit_subject(CONTENT, act);      // emit hook for rewritten subject
            break;
        case ATTRIBUTES:
            emit_attributes(CONTENT, act);   // emit hook for attributes
            break;

        default:
            break;
        }
        if (act->u.l.first != NULL || si == EQL) {    // mostly ignore empty lists
            if (act->u.l.first && act->u.l.first->type != FRAGELEM) {
                // record state generating this tree
                // - except for STRINGs which use state for quoting info
                act->state = si;
            }
        }
#endif
    return SUCCESS;
}
