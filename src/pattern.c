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
 * @param subject to be checked for pattern matches
 * @return newacts - with list of matched acts, or NULL
 */

// FIXME, this tacks a SUBJECT and returns a list of SUBJECTS
//      better if they were the same type
elem_t *
pattern(CONTENT_t * CONTENT, elem_t * subject)
{
    PARSE_t * PARSE = CONTENT->PARSE;
    LIST_t * LIST = (LIST_t *)PARSE;
    elem_t *newacts = NULL, *pattern_acts, *act, *subj, *attr;

    assert(subject);
    assert((state_t) subject->state == SUBJECT);

//E(LIST);
//P(LIST, subject);

    assert(CONTENT->subject_type == NODE || CONTENT->subject_type == EDGE);
    if (CONTENT->subject_type == NODE) {
        pattern_acts = CONTENT->node_pattern_acts;
    } else {
        pattern_acts = CONTENT->edge_pattern_acts;
    }

    // iterate over available patterns
    for ( act = pattern_acts->u.l.first; act; act = act->u.l.next) {
        assert((state_t) act->state == ACT);
        subj = act->u.l.first;
        assert(subj);
        assert((state_t) subj->state == SUBJECT);
        attr = subj->u.l.next;

        // FIXME - contents from pattern ??
        if ((match(CONTENT, subject->u.l.first, subj->u.l.first)) == SUCCESS) {
            // insert matched attrubutes, contents,
            // and then the subject again
            
            if (! newacts) {
                newacts = new_list(LIST, ACT);
            }
            append_list(newacts, ref_list(LIST, subject));
            emit_subject(CONTENT, subject);
            if (attr && (state_t)attr->state == ATTRIBUTES) {
                append_list(newacts, ref_list(LIST, attr));
                emit_attributes(CONTENT, attr);
            }

            // FIXME -- contents

            PARSE->stat_patternmatches++;
        }
    }

//E(LIST);
    return newacts;
}
