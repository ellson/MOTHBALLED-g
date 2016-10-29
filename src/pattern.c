/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <sys/types.h>
#include <assert.h>

#include "pattern.h"

/*
 * A pattern is a SUBJECT in which one or more STRINGs contain an AST ('*')
 * The AST is a wild-card that matches any substring of zero or more 
 * characters at that position.
 *
 * An indivual STRING may have no more that one AST, but multiple
 * STRING in the SUBJECT may have AST
 *
 * The following are valid pattern STRING:
 *      *       // This and the next require only prefix matching
 *      abcd*
 *
 *      ab*ef   // FIXME - this and the next require suffix matching - not yet implemented
 *      *cdef
 *
 * and all of these patterns will match
 *      abcdef
 * 
 * A SUBJECT is matched if all its pattern and non-pattern STRINGS
 * match, after pattern substitution.
 * 
 * ENDPOINTSETs are not expanded in patterns, or in SUBJECTs
 * before pattern matching. (i.e. the form of ENDPOINTSETS must be the same for a match to occur)
 */ 
 
/**
 * Look for pattern match(es) to the current subject (segregated
 * into NODE and EDGE patterns).
 * For each match, append a (refcounted copy) of the current
 * subject, followed by (refcounted) copies of the ATTRIBUTES
 * and CONTAINER from the pattern.  Finally return for the current
 * subject to be appended with its own ATTRIBUTES and ACTIVITY.
 *
 * @param CONTENT container_context
 * @param root of the output tree
 * @param subject to be checked for pattern matches
 */
void pattern(CONTENT_t * CONTENT, elem_t * root, elem_t * subject)
{
    PARSE_t * PARSE = CONTENT->PARSE;
    LIST_t * LIST = (LIST_t *)PARSE;
    elem_t *pattern_acts, *pact, *psubj, *pattr;

    assert(root);
    assert(subject);
    assert((state_t) subject->state == SUBJECT);

//E(LIST,"pattern1");

    assert(CONTENT->subject_type == NODE || CONTENT->subject_type == EDGE);
    if (CONTENT->subject_type == NODE) {
        pattern_acts = CONTENT->node_pattern_acts;
    } else {
        pattern_acts = CONTENT->edge_pattern_acts;
    }

    // iterate over available patterns
    for ( pact = pattern_acts->u.l.first; pact; pact = pact->next) {
        assert((state_t) pact->state == ACT);
        psubj = pact->u.l.first;
        assert(psubj);
        assert((state_t) psubj->state == SUBJECT);
        pattr = psubj->next;

        // FIXME - contents from pattern ??
        if ((match(CONTENT, subject->u.l.first, psubj->u.l.first)) == SUCCESS) {
            // insert matched attrubutes, contents,
            // and then the subject again
            
            append_list(root, ref_list(LIST, subject));
            emit_subject(CONTENT, subject);
            if (pattr && (state_t)pattr->state == ATTRIBUTES) {
                append_list(root, ref_list(LIST, pattr));
                emit_attributes(CONTENT, pattr);
            }

            // FIXME -- contents

            PARSE->stat_patternmatches++;
        }
    }

//E(LIST,"pattern2");
}
