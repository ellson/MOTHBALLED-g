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
 
void pattern_update(CONTAINER_t * CONTAINER, elem_t *act)
{
    THREAD_t *THREAD = CONTAINER->THREAD;   // needed for LIST() macro
    elem_t *act_tmp = act;

    if (CONTAINER->has_node) {
        CONTAINER->stat_patternnodecount++;
        CONTAINER->node_patterns =
            insert_item(LIST(), CONTAINER->node_patterns, &act_tmp, merge_pattern);
    } else {
        CONTAINER->stat_patternedgecount++;
        CONTAINER->edge_patterns =
            insert_item(LIST(), CONTAINER->edge_patterns, &act_tmp, merge_pattern);
    }
}

void pattern_remove(CONTAINER_t * CONTAINER, elem_t *act)
{
    THREAD_t *THREAD = CONTAINER->THREAD;   // needed for LIST() macro

    if (CONTAINER->has_node) {
        CONTAINER->stat_patternnodecount--;
        CONTAINER->node_patterns =
            remove_item(LIST(), CONTAINER->node_patterns, act);
    } else {
        CONTAINER->stat_patternedgecount--;
        CONTAINER->edge_patterns =
            remove_item(LIST(), CONTAINER->edge_patterns, act);
    }
}

/** 
 * Traverse the pattern tree in sort order, appending all
 * matching acts
 *
 * @param THREAD context
 * @param p the current tree elem
 * @param act to be appended with additional acts for matches
 */
static void pattern_match_r(THREAD_t* THREAD, elem_t *p, elem_t *act)
{
    if (p) {
 
// FIXME - it should be possible to optimize this to first search for the
// beginning on the range of matches, and then only the members of the range,
// instead of visiting all elements of the tree.

        if (p->u.t.left) {
            pattern_match_r(THREAD, p->u.t.left, act);
        }

P(act->u.l.first->u.l.first);
P(p->u.t.key->u.l.first->u.l.first);
        if (match(act->u.l.first->u.l.first, p->u.t.key->u.l.first->u.l.first) == 0) {
printf("MATCH\n");
P(p->u.t.key->u.l.first->u.l.next);
            append_addref(act, p->u.t.key->u.l.first->u.l.next);
        }

        if (p->u.t.right) {
            pattern_match_r(THREAD, p->u.t.right, act);
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
 * @param CONTAINER container_context
 * @param act to be checked for pattern matches
 * @return newacts - with list of matched acts, or NULL
 */

void pattern_match(CONTAINER_t * CONTAINER, elem_t * act)
{
    THREAD_t *THREAD = CONTAINER->THREAD;

    assert(act);
    assert((state_t) act->state == ACT);

    if (CONTAINER->has_node) {
        pattern_match_r(THREAD, CONTAINER->node_patterns, act);
    } else {
        pattern_match_r(THREAD, CONTAINER->edge_patterns, act);
    }
}
