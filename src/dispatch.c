/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

#include "dispatch.h"

static void
dispatch_r(LIST_t * LIST, elem_t * list, elem_t *attributes,
        elem_t * nodes, elem_t * edges);

static elem_t *
assemble_act(LIST_t * LIST, elem_t * elem, elem_t * attributes);

/*
 * Processes an ACT after sameas and pattern substitutions.
 *
 *
 * FIXME - description needs updating
 *
 * - expand OBJECT_LIST
 *    - expand ENDPOINT_SETs
 *       - promote ENDPOINTS to common ancestor
 *           - extract NODES from EDGES for node induction
 *               - dispatch NODES (VERB NODE ATTRIBUTES CONTENT)
 *                   - if add or delete and CONTENT is from pattern, copy content before modifying (COW)
 *           - dispatch EDGE  (VERB EDGE ATTRIBUTES CONTENT)
 *               - if add or delete and CONTENT is from pattern, copy content before modifying (COW)
 *
 * VERBs (add), delete, query
 *
 * Edges are owned (stored, rendered) by the common ancestor
 * of all the endpoints.
 *
 * So an edge to an external node:, e.g
 *                      <sibling ^/uncle/cousin>
 * is dealt with as follows:    // FIXME - not right
 *       1. node "sibling" is induced (created in the local
 *              graph of siblings, if it isn't already there.)
 *       2. the edge is sent though a special channel to
 *              the nameless parent of the the current siblings.
 *              That parent, prepends its name to all noderefs
 *              without a ^/, and removes one ^/ from all the others,
 *              So. in the parent's graph, the edge becomes:
 *                      <mum/sibling uncle/cousin>
 *              If there still ^/ left, then the process
 *              is repeated, sending it though the nameless channel to
 *              their parent, which applies the same rule:
 */

/**
 * @param CONTENT container context
 * @param an ACT
 * @return new list of ACTS
 */
elem_t *
dispatch(CONTENT_t * CONTENT, elem_t * act)
{
    PARSE_t *PARSE = CONTENT->PARSE;
    LIST_t * LIST = (LIST_t *)PARSE;
    state_t si;
    elem_t *new, *newacts, *elem, *nodes, *edges, *attributes;
    
    assert(act);
    assert(act->type == (char)LISTELEM);
    assert((state_t) act->state == ACT);

//E(LIST);
//P(LIST, act);

    newacts = new_list(LIST, 0); // return list

    nodes = new_list(LIST, 0);
    edges = new_list(LIST, 0);
    attributes = new_list(LIST, 0);

    // expand OBJECT_LIST and ENDPOINTSETS
    dispatch_r(LIST, act, attributes, nodes, edges);

    // if NODE ACT ... for each NODE from nodes, generate new ACT: verb node attributes
    // else if EDGE ACT ... for each NODEREF, generate new ACT: verb node
    //                      for each EDGE, generate new ACT: verb edge attributes

    si = CONTENT->subject_type;
    switch (si) {
    case NODE:
        elem = nodes->u.l.first;
        while (elem) {
            new = assemble_act(LIST, elem, attributes);
            append_list_move(newacts, new);
            elem = elem->u.l.next;
        }
        break;
    case EDGE:
        if (PARSE->has_cousin) {
            // FIXME - deal with edges that require help from ancestors
            fprintf(stdout,"EDGE has COUSIN\n");
        }
        else {
                elem = nodes->u.l.first;
                while (elem) {
                    // inducing nodes from NODEREFS - no attriibutes from these
                    new = assemble_act(LIST, elem, NULL);
                    append_list_move(newacts, new);
                    elem = elem->u.l.next;
                }

                elem = edges->u.l.first;
                while (elem) {
                    new = assemble_act(LIST, elem, attributes);
                    append_list_move(newacts, new);
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

//E(LIST);
//P(LIST, newacts);
    return(newacts);
}

/**
 * This function expands OBJECT_LIST of NODES or EDGES,
 *     then expands ENPOINTSETS in EDGES,
 *     and returns the expansion as list of simple nodes and edges
 *
 * @param LIST context
 * @param list of object -- not modified
 * @param attributes -- appended
 * @param nodes -- appended
 * @param edges -- appended
 */
static void
dispatch_r(LIST_t * LIST, elem_t * list, elem_t * attributes,
        elem_t * nodes, elem_t * edges)
{
    elem_t *elem, *new, *object;
    state_t si1, si2;

    assert(list->type == (char)LISTELEM);
//E(LIST);
//P(LIST, list);

    elem = list->u.l.first;
    while (elem) {
        si1 = (state_t) elem->state;
        switch (si1) {
        case ACT:
            dispatch_r(LIST, elem, attributes, nodes, edges);
            break;
        case ATTRIBUTES:
            new = ref_list(LIST, elem);
            append_list_move(attributes, new);
            break;
        case SUBJECT:
            object = elem->u.l.first;
            si2 = (state_t)object->state;
            switch (si2) {
            case OBJECT:
                dispatch_r(LIST, object, attributes, nodes, edges);
                break;
            case OBJECT_LIST:
                object = object->u.l.first;
                while(object) {
                    assert((state_t)object->state == OBJECT);
                    dispatch_r(LIST, object, attributes, nodes, edges);
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
            append_list_move(nodes, new);
            break;
        case EDGE:
            expand(LIST, elem, nodes, edges);
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
//E(LIST);
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
assemble_act(LIST_t * LIST, elem_t *elem, elem_t *attributes)
{
    TOKEN_t * IN = (TOKEN_t *)LIST;
    elem_t *new;
    elem_t *act;

    act = new_list(LIST, ACT);  // return list

    // verb
    switch(IN->verb) {
    case QRY:
    case TLD:
        new = new_list(LIST, IN->verb);
        append_list_move(act, new);
        break;
    case 0:  // default is add
        break;
    default:
        assert(0); //should never get here
        break;
    }

    // subject
    new = ref_list(LIST, elem);
    append_list_move(act, new);

    // attributes
    if (attributes && attributes->u.l.first) {
        new = ref_list(LIST, attributes->u.l.first);
        append_list_move(act, new);
    }

    // no container ever because contents are handled in a parse_nest_r() recursion.

    return act;
}
