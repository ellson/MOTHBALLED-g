/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <sys/types.h>
#include <assert.h>

#include "thread.h"
#include "match.h"
#include "compare.h"
#include "pattern.h"

static void pattern_update(CONTAINER_t * CONTAINER, elem_t * subject, state_t verb);
static elem_t * pattern_match(CONTAINER_t * CONTAINER, elem_t * subject);

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
 

elem_t * pattern(CONTAINER_t * CONTAINER, elem_t * subject, state_t verb)
{
    GRAPH_t *GRAPH = (GRAPH_t*)CONTAINER;
    elem_t *newsubjects;

    if ((CONTAINER->is_pattern = ((TOKEN_t*)GRAPH)->is_pattern)) {
        GRAPH->stat_patternactcount++;
        assert(CONTAINER->subject_type == NODE || CONTAINER->subject_type == EDGE);
        pattern_update(CONTAINER, subject, verb);
        return NULL;
    }
    GRAPH->stat_nonpatternactcount++;
    newsubjects = pattern_match(CONTAINER, subject);
    return newsubjects;
}

static void pattern_update(CONTAINER_t * CONTAINER, elem_t * subject, state_t verb)
{
    assert(CONTAINER->subject_type == NODE || CONTAINER->subject_type == EDGE);
    if (verb == (char)QRY) {
        assert(0);  // FIXME - report error
    }
    if (CONTAINER->subject_type == NODE) {
        if (verb == (char)TLD) {
 //           remove_item(LIST, CONTAINER->node_pattern_acts, obj);
        } else {
 //           insert_item(LIST, CONTAINER->node_pattern_acts, obj);
            append_transfer(CONTAINER->node_pattern_acts, subject);
        }
    } else {
        if (verb == (char)TLD) {
 //           remove_item(LIST, CONTAINER->edge_pattern_acts, obj);
        } else {
 //           insert_item(LIST, CONTAINER->edge_pattern_acts, obj);
            append_transfer(CONTAINER->edge_pattern_acts, subject);
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
 * @param CONTAINER container_context
 * @param subject to be checked for pattern matches
 * @return newacts - with list of matched acts, or NULL
 */

static elem_t * pattern_match(CONTAINER_t * CONTAINER, elem_t * subject)
{
    GRAPH_t * GRAPH = (GRAPH_t*)CONTAINER;
    LIST_t * LIST = (LIST_t *)GRAPH;
    elem_t *newacts = NULL, *pattern_acts, *subj, *attr;

    assert(subject);
    assert((state_t) subject->state == SUBJECT);

//E();
//P(subject);

    assert(CONTAINER->subject_type == NODE || CONTAINER->subject_type == EDGE);
    if (CONTAINER->subject_type == NODE) {
        pattern_acts = CONTAINER->node_pattern_acts;
    } else {
        pattern_acts = CONTAINER->edge_pattern_acts;
    }

    // iterate over available patterns
    for ( subj = pattern_acts->u.l.first; subj; subj = subj->u.l.next) {
        assert(subj);
        assert((state_t) subj->state == SUBJECT);
        attr = subj->u.l.next;

        // FIXME - contents from pattern ??
        if ((match(CONTAINER, subject->u.l.first, subj->u.l.first)) == SUCCESS) {
            // insert matched attrubutes, contents,
            // and then the subject again
            
            if (! newacts) {
                newacts = new_list(LIST, ACT);
            }
            append_addref(newacts, ref_list(LIST, subject));
            if (attr && (state_t)attr->state == ATTRIBUTES) {
                append_addref(newacts, ref_list(LIST, attr));
            }

            // FIXME -- contents

            GRAPH->stat_patternmatches++;
        }
    }

//E();
    return newacts;
}

