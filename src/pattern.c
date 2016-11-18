/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <sys/types.h>
#include <assert.h>

#include "thread.h"
#include "merge.h"
#include "compare.h"
#include "pattern.h"

/*
 * A pattern is an ACT in which one or more IDENTIFIERs contain an AST ('*')
 * The AST is a wild-card that matches any substring of zero or more 
 * characters at that position.
 *
 * An indivual IDENTIFIER may have no more that one AST, but multiple
 * IDENTIFIER in the ACT may have AST
 *
 * The following are valid pattern IDENTIFIER:
 *      *       // This and the next require only prefix matching
 *      abcd*   
 *      abcdef  // Exact matches are ok
 *
 * The following are not (yet) supported because they requre suffix matching
 *      *def
 *      ab*ef
 * 
 * An ACT is matched if all its pattern and non-pattern IDENTIFIERs
 * match, after pattern substitution.
 */ 
 
static void pattern_update(CONTAINER_t * CONTAINER, elem_t *act)
{
    THREAD_t *THREAD = CONTAINER->THREAD;   // needed for LIST() macro

    if (CONTAINER->has_node) {
        CONTAINER->stat_patternnodecount++;
        CONTAINER->node_patterns =
            insert_item(LIST(), CONTAINER->node_patterns, act->u.l.first, merge_pattern, NULL);
    } else {
        CONTAINER->stat_patternedgecount++;
        CONTAINER->edge_patterns =
            insert_item(LIST(), CONTAINER->edge_patterns, act->u.l.first, merge_pattern, NULL);
    }
}

static void pattern_remove(CONTAINER_t * CONTAINER, elem_t *act)
{
    THREAD_t *THREAD = CONTAINER->THREAD;   // needed for LIST() macro

    if (CONTAINER->has_node) {
        CONTAINER->stat_patternnodecount--;
        CONTAINER->node_patterns =
            remove_item(LIST(), CONTAINER->node_patterns, act->u.l.first);
    } else {
        CONTAINER->stat_patternedgecount--;
        CONTAINER->edge_patterns =
            remove_item(LIST(), CONTAINER->edge_patterns, act->u.l.first);
    }
}

/** 
 * Traverse the pattern tree in sort order, appending all
 * matching acts
 *
 * @param THREAD context
 * @param p the current tree elem
 * @param subject to be matched
 * @param attributes  to be appended with matches
 */
static void pattern_match_r(THREAD_t* THREAD, elem_t *p, elem_t *subject, elem_t *attributes)
{
    if (p) {
 
// FIXME - it should be possible to optimize this to first search for the
// beginning on the range of matches, and then only the members of the range,
// instead of visiting all elements of the tree.
// Except that matching is expensive, so don't match more than once.
// Perhaps stack matches on the way to the beginning, then play them back...

        if (p->u.t.left) {
            pattern_match_r(THREAD, p->u.t.left, subject, attributes);
        }

//P(subject->u.l.first);
//P(p->u.t.key->u.l.first);
        if (match(subject->u.l.first, p->u.t.key->u.l.first) == 0) {
            elem_t *attr = p->u.t.key->u.l.next->u.l.first;
            while (attr) {
                elem_t *new = new_list(LIST(), ATTR);
                append_addref(new, attr->u.l.first);
                append_transfer(attributes, new);
                attr = attr->u.l.next;
            }
        }

        if (p->u.t.right) {
            pattern_match_r(THREAD, p->u.t.right, subject, attributes);
        }
    }
}

/**
 * Look for pattern match(es) to the current ACT 
 * For each match, append a (refcounted copy) of the current
 * subject, followed by (refcounted) copies of the ATTRIBUTES
 * and CONTAINER from the pattern.  Finally return for the current
 * subject to be appended with its own ATTRIBUTES and ACTIVITY.
 *
 * @param CONTAINER context
 * @param act to be checked for pattern matches
 * @param attributes to be appended if the pattern is matched
 * @return newacts - with list of matched acts, or NULL
 */

static void pattern_match(CONTAINER_t * CONTAINER, elem_t * act, elem_t *attributes)
{
    THREAD_t *THREAD = CONTAINER->THREAD;

    if (CONTAINER->has_node) {
        pattern_match_r(THREAD, CONTAINER->node_patterns, act->u.l.first, attributes);
    } else {
        pattern_match_r(THREAD, CONTAINER->edge_patterns, act->u.l.first, attributes);
    }
}



/** 
 * If the act is a pattern with attributes, then store it in the pattern tree and return NULL.
 * If the act is a pattern with no attributes, then remove it from the pattern tree and return NULL.
 * If the act is a non-pattern act, the search the store for matching patterns, apply them,
 *    and return a rewritten act.
 *
 * @param CONTAINER context
 * @param act to be processed for patterns
 * @return a replacement act, or NULL
 */
elem_t * patterns(CONTAINER_t *CONTAINER, elem_t *act)
{
    THREAD_t *THREAD = CONTAINER->THREAD;
    elem_t *subject, *attributes;
    elem_t *newact, *newsubject, *newattributes;
 
    assert(act);
    subject = act->u.l.first;
    assert(subject);                // minimaly, an ACT must have a SUBJECT
    attributes = subject->u.l.next; // may be NULL

    if ( !CONTAINER->verb) { // if verb is default (add)
        if (CONTAINER->subj_has_ast) {
            if (attributes) {
                // if the pattern act has attributes, then it is added to saved patterns
                // (if if the pattern subject is already saved, then it is replaced
                // by this new one)
                pattern_update(CONTAINER, act);
            } else {
                // if the pattern has no attributes then it is removed from saved patterns
                // N.B. This is how patterns are deleted
                pattern_remove(CONTAINER, act);
            }
            return NULL;  // new pattern stored,  no more processing for this ACT
        }
    }

    // Now we are going to build a rewritten ACT tree, with references
    // to various bits from the parser's tree,  but no changes to it.
    //
    // In particular,  we must use fresh ACT, SUBJECT, ATTRIBUTE lists.

    newact = new_list(LIST(), ACT);
    newsubject = new_list(LIST(), SUBJECT);
    append_addref(newsubject, subject->u.l.first);
    append_transfer(newact, newsubject);
    newattributes = NULL;

    // append pattern attrs, if any
    if ( !CONTAINER->verb) { // if verb is default (add)
        newattributes = new_list(LIST(), ATTRIBUTES);
        pattern_match(CONTAINER, act, newattributes);
        if (!newattributes->u.l.first) {
            free_list(LIST(), newattributes);
            newattributes = NULL;
        }
    }
    // append current attr, if any, after pattern_match so that
    // attr from patterns can be over-ridden
    if (attributes) {
        if (!newattributes) {
            newattributes = new_list(LIST(), ATTRIBUTES);
        }
        append_addref(newattributes, attributes->u.l.first);
    }
    if (newattributes) {
        append_transfer(newact, newattributes);
    }

    // patterns now applied for "add"  verb - may now have multiple ACTs
    // may still have subj_has_ast in QRY or TLD

    return newact;
}
