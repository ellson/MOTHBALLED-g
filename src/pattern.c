/* vim:set shiftwidth=4 ts=8 expandtab: */

/*************************************************************************
 * Copyright (c) 2017 AT&T Intellectual Property
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: John Ellson <john.ellson@gmail.com>
 *************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <sys/types.h>
#include <assert.h>

#include "thread.h"
#include "pattern.h"


// FIXME - currently patterns apply to future node or edges.
//       - this should be changed to apply to existing nodes or edges.

// For example:    a* { c [d=e] }

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
 
/**
 * merge a new pattern act
 * by replacing old ATTRIBUTES with ATTRIBUTES from the new pattern act.
 *
 * @param LIST 
 * @param act - the new (replacement) pattern act
 * @param key - the key with the old pattern act
 * @return    - the key with the old pattern act
 */
static elem_t * merge_pattern(LIST_t *LIST, elem_t *act, elem_t *key)
{
    // free old ATTRIBUTES and append new ATTRIBUTES
    free_list(LIST, key->u.l.first->u.l.next);
    key->u.l.first->u.l.next = ref_list(LIST, act->u.l.first->u.l.next);
    return key;
}

static void pattern_update(CONTAINER_t * CONTAINER, elem_t *act)
{
    THREAD_t *THREAD = CONTAINER->THREAD;   // needed for LIST() macro

    if (CONTAINER->has_node) {
        CONTAINER->stat_patternnodecount++;
        CONTAINER->node_patterns =
            insert_item(LIST(), CONTAINER->node_patterns,
                act->u.l.first, merge_pattern, NULL);
    } else {
        CONTAINER->stat_patternedgecount++;
        CONTAINER->edge_patterns =
            insert_item(LIST(), CONTAINER->edge_patterns,
                act->u.l.first, merge_pattern, NULL);
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
// beginning of the range of matches, and then only the members of the range,
// instead of visiting all elements of the tree.
// Stack matches on the way to the leftmost node >= the pattern, then play them back while =,
// then continue to the right while <= the pattern.

        if (p->u.t.left) {
            pattern_match_r(THREAD, p->u.t.left, subject, attributes);
        }

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
    elem_t *subject, *attributes, *disambig, *disambid = NULL;
    elem_t *newact, *newsubject, *newattributes;
 
    assert(act);
    subject = act->u.l.first;
    assert(subject);                // minimaly, an ACT must have a SUBJECT
    disambig = subject->u.l.next;   // may be NULL or may be ATTRIBUTES
    if (disambig && (state_t)disambig->state == DISAMBIG) {
        attributes = disambig->u.l.next;  // may be NULL
        disambid = disambig->u.l.first;
    } else {
        attributes = disambig;   // wasn't disambig
        disambig = NULL;
    }
    if (THREAD->contenthash[0]) {
        elem_t *newattr, *newattrid, *newvalue, *newident;
        // Build newattributes for "_contenthash=xxxxx"
        // FIXME - need a litle-language to simplify
        //     these constructions
        newattr = new_list(LIST(), ATTR);
        newattrid = new_list(LIST(), ATTRID);
        append_transfer(newattr, newattrid);
        newident = new_shortstr(LIST(), ABC, "_contenthash");
        append_transfer(newattrid, newident);
        newvalue = new_list(LIST(), VALUE);
        append_transfer(newattr, newvalue);
        newident = new_shortstr(LIST(), ABC, THREAD->contenthash);
        append_transfer(newvalue, newident);
        if (attributes) { // append to existing attibutes
            append_transfer(attributes, newattr);
        } else {
            newattributes = new_list(LIST(), ATTRIBUTES);
            append_transfer(newattributes, newattr);
            append_transfer(act, newattributes);
        }
    }
    if ( !CONTAINER->verb) { // if verb is default (add)
        if (CONTAINER->subj_has_ast) {
            if (attributes || THREAD->contenthash[0]) {
                // If the pattern act has attributes and/or contents,
                // then it is added to saved patterns.
                // If the pattern subject is already saved,
                // then it is replaced by this new one.
                pattern_update(CONTAINER, act);
            } else {
                // if the pattern has no attributes and no content
                // hen it is removed from saved patterns
                // N.B. This is how patterns are deleted
                pattern_remove(CONTAINER, act);
            }
            return NULL;  // new pattern stored,
                          //   no more processing for this ACT
        }
    }
    THREAD->contenthash[0] = '\0';

    // Now we are going to build a rewritten ACT tree, with references
    // to various bits from the parser's tree,  but no changes to it.
    //
    // In particular,  we must use fresh ACT, SUBJECT, DISAMBIG, ATTRIBUTE lists.

    newact = new_list(LIST(), ACT);
    newsubject = new_list(LIST(), SUBJECT);
    append_addref(newsubject, subject->u.l.first);
    append_transfer(newact, newsubject);

    // append disambig, if any
    if (disambig) {
        elem_t *newdisambig = new_list(LIST(), DISAMBIG);
        append_addref(newdisambig, disambid);
        append_transfer(newact, newdisambig);
    }

    // append pattern attributes, if any
    newattributes = NULL;
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
