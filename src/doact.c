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
    elem_t *subj, *obj, *attr, *valu, *elem, *new;
    state_t si, verb = 0;

    assert(act);
    assert(act->u.l.first);  // minimaly, an ACT must have a SUBJECT


    PARSE->stat_inactcount++;

P(LIST, act);



//====================== VERB
    subj = act->u.l.first;             // tentatively, first item is SUBJECT
    if (subj->state == (char)VERB) {   // but it might be a VERB instead
        verb = subj->u.l.first->state; // record TLD '~' for: "delete"
                                       //     or QRY '?' for: "query"
                                       //        defaults to: "add"
 
        subj = subj->u.l.next;         // SUBJECT must be second item
    }
    assert(subj);
    assert(subj->state == (char)SUBJECT);
//----------------------- example
// G:      ?a
//
// Parse:  ACT VERB QRY
//             SUBJECT OBJECT NODE NODEID ABC a
//
// Here:   SUBJECT OBJECT NODE NODEID ABC a
// P(LIST, subj);

   

//======================= classify NODE or EDGE, and perform sameas substitions
    obj = sameas(CONTENT, subj);
    assert(obj);
//----------------------- example
// G:      <a b> <= c>
//
// Parse:  ACT SUBJECT OBJECT EDGE LAN
//                                 LEG ENDPOINT SIBLING NODEREF NODEID ABC a
//                                 LEG ENDPOINT SIBLING NODEREF NODEID ABC b
//                                 RAN
//         ACT SUBJECT OBJECT EDGE LAN
//                                 LEG EQL
//                                 LEG ENDPOINT SIBLING NODEREF NODEID ABC c
//                                 RAN
//
// Here:   SUBJECT OBJECT EDGE LAN
//                             LEG ENDPOINT SIBLING NODEREF NODEID ABC a
//                             LEG ENDPOINT SIBLING NODEREF NODEID ABC b
//                             RAN
//         SUBJECT OBJECT EDGE LAN
//                             LEG ENDPOINT SIBLING NODEREF NODEID ABC a
//                             LEG ENDPOINT SIBLING NODEREF NODEID ABC c
//                             RAN
//----------------------- 
//P(LIST, obj);



//======================= process ATTRIBUTES (if any)
    attr = subj->u.l.next;
    if (attr) {
        // ATTRID are stored in a sorted list of all ATTRID encountered
        // VALUE are stored in a list sorted by ATTRID for this ACT
        elem = attr->u.l.first;
        while (elem) {
            si = (state_t)elem->state;
            switch (si) {
            case LBR:
            case RBR:
                break; // ignore
            case ATTR:
                // don't look!
                attr = elem->u.l.first;
                valu = attr->u.l.next->u.l.first->u.l.next->u.l.first;
                attr = attr->u.l.first;
                // phew!
P(LIST, attr);
P(LIST, valu);
                // FIXME - need a version that keeps old on match
                if (CONTENT->subject_type == NODE) {
//                    CONTENT->node_attrid = insert_item(LIST, CONTENT->node_attrid, attr);
                } else {
//                    CONTENT->edge_attrid = insert_item(LIST, CONTENT->edge_attrid, attr);
                }
                break;
            default:
                S(si);
                assert(0); // shouldn't be here
                break;
            }
            elem = elem->u.l.next;
        }
    }
//----------------------- example
// G:       a[foo=bar abc=xyz]
//
// Parse:   ACT SUBJECT OBJECT NODE NODEID ABC a
//              ATTRIBUTES LBR
//                         ATTR ATTRID ABC foo
//                              VALASSIGN EQL
//                                        VALUE ABC bar
//                         ATTR ATTRID ABC abc
//                              VALASSIGN EQL
//                                        VALUE ABC xyz
//                         RBR
//
// Here:    SUBJECT OBJECT NODE NODEID ABC a
//----------------------- 
//P(LIST, obj);



//======================= collect, remove, or apply patterns
    if (CONTENT->is_pattern) {
        PARSE->stat_patternactcount++;
        assert(CONTENT->subject_type == NODE || CONTENT->subject_type == EDGE);
        pattern_update(CONTENT, obj, verb);
        return SUCCESS;
    } 
    PARSE->stat_nonpatternactcount++;
    new = pattern_match(CONTENT, obj);
    if (new) {
        free_list(LIST, obj);
        obj = new;
    }
    //  N.B. (there can be multiple obj after pattern subst.  Each matched
    //  pattern generates an additional object.

    assert(obj);

#if 0
// FIXME so this is probably flawed - doesn't it need a loop?
    // dispatch events for the ACT just finished
    new = dispatch(CONTENT, outacts);
    if (new) {
        free_list(LIST, outacts);
        outacts = new;
    }

    elem = outacts->u.l.first;
    while (elem) {
        PARSE->stat_outactcount++;
//P(LIST,elem);
        reduce(CONTENT, elem);  // eliminate reduncy by insertion sorting into trees.

        elem = elem->u.l.next;
    }
#endif

    free_list(LIST, obj);

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
