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

/*
 * A pattern is an ACT in which one or more STRINGs contain an AST ('*')
 * The AST is a wild-card that matches any substring of zero or more 
 * characters at that position.
 *
 * An indivual STRING may have no more that one AST, but multiple
 * STRING in the ACT may have AST
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
 * An ACT is matched if all its pattern and non-pattern STRINGS
 * match, after pattern substitution.
 */ 
 
void pattern_update(CONTAINER_t * CONTAINER, elem_t * act)
{
    switch(CONTAINER->verb) {
        case 0:
            CONTAINER->stat_patternactcount++;
            append_addref(CONTAINER->patterns, act);
            break;
        case QRY:
            assert(0);  // FIXME - report error
            break;
        case TLD:
            assert(0);  // FIXME - report error
            break;
        default:
            assert(0);
            break;
    }
}

/**
 * Look for pattern match(es) to the current ACT 
 * For each match, append a (refcounted copy) of the current
 * subject, followed by (refcounted) copies of the ATTRIBUTES
 * and CONTAINER from the pattern.  Finally return for the current
 * subject to be appended with its own ATTRIBUTES and ACTIVITY.
 *
 * @param CONTAINER container_context
 * @param act to be checked for pattern matches
 * @return newacts - with list of matched acts, or NULL
 */

elem_t * pattern_match(CONTAINER_t * CONTAINER, elem_t * act)
{
    THREAD_t *THREAD = CONTAINER->THREAD;
    elem_t *newacts = NULL, *pattern_acts, *subj, *attr;

    assert(act);
    assert((state_t) act->state == ACT);

//E();
//P(act);

    // iterate over available patterns
    for ( subj = CONTAINER->patterns->u.l.first; subj; subj = subj->u.l.next) {
        assert(subj);
        assert((state_t) subj->state == SUBJECT);
        attr = subj->u.l.next;

        // FIXME - contents from pattern ??
        if ((match(CONTAINER, act->u.l.first, subj->u.l.first)) == SUCCESS) {
            // insert matched attrubutes, contents,
            // and then the subject again
            
            if (! newacts) {
                newacts = new_list(LIST(), ACT);
            }
            append_addref(newacts, ref_list(LIST(), act));
            if (attr && (state_t)attr->state == ATTRIBUTES) {
                append_addref(newacts, ref_list(LIST(), attr));
            }

            // FIXME -- contents

            CONTAINER->stat_patternmatches++;
        }
    }

//E();
    return newacts;
}

