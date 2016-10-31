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
 *      abcdef  // Exact matches are ok
 *
 * The following are not (yet) supported because they requre suffix matching
 *      *def
 *      ab*ef
 * 
 * A SUBJECT is matched if all its pattern and non-pattern STRINGS
 * match, after pattern substitution.
 * 
 * The form of ENDPOINTSETS must be the same for a match to occur.
 */ 
 

void pattern_update(CONTENT_t * CONTENT, elem_t * subject, state_t verb)
{
    assert(CONTENT->subject_type == NODE || CONTENT->subject_type == EDGE);
    if (verb == (char)QRY) {
        assert(0);  // FIXME - report error
    }
    if (CONTENT->subject_type == NODE) {
        if (verb == (char)TLD) {
 //           remove_item(LIST, CONTENT->node_pattern_acts, obj);
        } else {
 //           insert_item(LIST, CONTENT->node_pattern_acts, obj);
            append_transfer(CONTENT->node_pattern_acts, subject);
        }
    } else {
        if (verb == (char)TLD) {
 //           remove_item(LIST, CONTENT->edge_pattern_acts, obj);
        } else {
 //           insert_item(LIST, CONTENT->edge_pattern_acts, obj);
            append_transfer(CONTENT->edge_pattern_acts, subject);
        }
    }
}

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
pattern_match(CONTENT_t * CONTENT, elem_t * subject)
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
            append_addref(newacts, ref_list(LIST, subject));
            emit_subject(CONTENT, subject);
            if (attr && (state_t)attr->state == ATTRIBUTES) {
                append_addref(newacts, ref_list(LIST, attr));
                emit_attributes(CONTENT, attr);
            }

            // FIXME -- contents

            PARSE->stat_patternmatches++;
        }
    }

//E(LIST);
    return newacts;
}

