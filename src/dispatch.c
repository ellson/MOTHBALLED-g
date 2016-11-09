/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

#include "thread.h"
#include "expand.h"
#include "dispatch.h"

static void
dispatch_r(CONTAINER_t * CONTAINER, elem_t * list, elem_t *attributes,
        elem_t * nodes, elem_t * edges, state_t verb);

static elem_t *
assemble_act(LIST_t * LIST, elem_t * elem, elem_t * attributes, state_t verb);

/*
 * Processes an ACT after sameas and pattern substitutions.
 *
 *
 * FIXME - description needs updating
 *
 * - expand NOUNSET
 *    - expand ENDPOINT_SETs
 *       - promote ENDPOINTS to common ancestor
 *           - extract NODES from EDGES for node induction
 *               - dispatch NODES (VERB NODE ATTRIBUTES CONTAINER)
 *                   - if add or delete and CONTAINER is from pattern, copy content before modifying (COW)
 *           - dispatch EDGE  (VERB EDGE ATTRIBUTES CONTAINER)
 *               - if add or delete and CONTAINER is from pattern, copy content before modifying (COW)
 *
 * VERBs (add), delete, query
 *
 * Edges are owned (stored, rendered) by the common ancestor
 * of all the endpoints.
 *
 * So an edge to an external node:, e.g
 *                      <sis ^aunt/cousin>
 * is dealt with as follows:    // FIXME - not right
 *       1. node "sibling" is induced (created in the local
 *              graph of siblings, if it isn't already there.)
 *       2. the edge is sent though a special channel to
 *              the nameless parent (mum) of the the current siblings.
 *              That parent, prepends its name to all noderefs
 *              without a ^, and removes one ^ from all the others,
 *              So. in the parent's graph, the edge becomes:
 *                      <mum/sis aunt/cousin>
 *              If there still ^ left, then the process
 *              is repeated, sending it though the nameless channel to
 *              their parent, which applies the same rule:
 */

/**
 * @param CONTAINER container context
 * @param an ACT
 * @param verb - add, del, qry 
 * @return new list of ACTS
 */
elem_t *
dispatch(CONTAINER_t * CONTAINER, elem_t * act, state_t verb)
{
    PARSE_t *PARSE = (PARSE_t*)CONTAINER;
    THREAD_t *THREAD = CONTAINER->THREAD;
    LIST_t * LIST = (LIST_t *)THREAD;
    state_t si;
    elem_t *new, *newacts, *elem, *nodes, *edges, *attributes;
    
    assert(act);
    assert(act->type == (char)LISTELEM);
    assert((state_t) act->state == ACT);

//E();
//P(act);

    newacts = new_list(LIST, 0); // return list

    nodes = new_list(LIST, 0);
    edges = new_list(LIST, 0);
    attributes = new_list(LIST, 0);

    // expand NOUNSET and ENDPOINTSETS
    dispatch_r(CONTAINER, act, attributes, nodes, edges, verb);

    // if NODE ACT ... for each NODE from nodes, generate new ACT: verb node attributes
    // else if EDGE ACT ... for each NODEREF, generate new ACT: verb node
    //                      for each EDGE, generate new ACT: verb edge attributes

    si = CONTAINER->subject_type;
    switch (si) {
    case NODE:
        elem = nodes->u.l.first;
        while (elem) {
            new = assemble_act(LIST, elem, attributes, verb);
            append_transfer(newacts, new);
            elem = elem->u.l.next;
        }
        break;
    case EDGE:
        if (PARSE->need_mum) {
            // FIXME - deal with edges that require help from ancestors
            fprintf(stdout,"EDGE has COUSIN\n");
        }
        else {
                elem = nodes->u.l.first;
                while (elem) {
                    // inducing nodes from NODEREFS - no attributes from these
                    new = assemble_act(LIST, elem, NULL, verb);    //FIXME we dont want to delete induced nodes!!
                    append_transfer(newacts, new);
                    elem = elem->u.l.next;
                }

                elem = edges->u.l.first;
                while (elem) {
                    new = assemble_act(LIST, elem, attributes, verb);
                    append_transfer(newacts, new);
                    elem = elem->u.l.next;
                }
        }
        break;
    default:
        S(si);
        assert(0);  // shouldn't happen
        break;
    }

    free_list(LIST, nodes);
    free_list(LIST, edges);
    free_list(LIST, attributes);

//E();
//P(newacts);
    return(newacts);
}

/**
 * This function expands NOUNSET of NODES or EDGES,
 *     then expands ENPOINTSETS in EDGES,
 *     and returns the expansion as list of simple nodes and edges
 *
 * @param LIST context
 * @param list of object -- not modified
 * @param attributes -- appended
 * @param nodes -- appended
 * @param edges -- appended
 * @param verb
 */
static void
dispatch_r(CONTAINER_t * CONTAINER, elem_t * list, elem_t * attributes,
        elem_t * nodes, elem_t * edges, state_t verb)
{
    THREAD_t *THREAD = CONTAINER->THREAD;
    LIST_t *LIST = (LIST_t*)THREAD;
    elem_t *elem, *new, *object;
    state_t si1, si2;

    assert(list->type == (char)LISTELEM);
//E();
//P(list);

    elem = list->u.l.first;
    while (elem) {
        si1 = (state_t) elem->state;
        switch (si1) {
        case ACT:
            dispatch_r(CONTAINER, elem, attributes, nodes, edges, verb);
            break;
        case ATTRIBUTES:
            new = ref_list(LIST, elem);
            append_transfer(attributes, new);
            break;
        case SUBJECT:
            object = elem->u.l.first;
            si2 = (state_t)object->state;
            switch (si2) {
            case NOUN:
                dispatch_r(CONTAINER, object, attributes, nodes, edges, verb);
                break;
            case NOUNSET:
                object = object->u.l.first;
                while(object) {
                    assert((state_t)object->state == NOUN);
                    dispatch_r(CONTAINER, object, attributes, nodes, edges, verb);
                    object = object->u.l.next;
                }
                break;
            default:
                S(si2);
                assert(0); //should never get here
                break;
            }
            break;
        case NODE:
            new = ref_list(LIST, elem);
            append_transfer(nodes, new);
            break;
        case EDGE:
            expand(CONTAINER, elem, nodes, edges);
            break;
        case VERB:  // ignore - already dealt with
            break;
        default:
            S(si1);
            assert(0);  // should never get here
            break;
        }
        elem = elem->u.l.next;
    }
//E();
}

/**
 * This function assembles ACTs with simplified SUBJECT and no CONTAINER
 * The act VERB is copied from the current input ACTIVITY verb
 *
 * @param LIST context
 * @param elem   -- node or edge object -- not modified
 * @param attributes -- not modified
 * @return a node act
 */
static elem_t *
assemble_act(LIST_t * LIST, elem_t *elem, elem_t *attributes, state_t verb)
{
    elem_t *new, *act;

    act = new_list(LIST, ACT);  // return list

    // verb
    switch(verb) {
    case QRY:
    case TLD:
        new = new_list(LIST, verb);
        append_transfer(act, new);
        break;
    case 0:  // default is add
        break;
    default:
        assert(0); //should never get here
        break;
    }

    // subject
    new = ref_list(LIST, elem);
    append_transfer(act, new);

    // attributes
    if (attributes && attributes->u.l.first) {
        new = ref_list(LIST, attributes->u.l.first);
        append_transfer(act, new);
    }

    // no container ever because contents are handled in a parse_nest_r() recursion.

    return act;
}
