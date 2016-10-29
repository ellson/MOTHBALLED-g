/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

#include "dispatch.h"

static void
dispatch_r(LIST_t * LIST, elem_t * list, elem_t *attributes,
        elem_t * nodes, elem_t * edges);

static void
assemble_act(LIST_t * LIST, elem_t * elem, elem_t * attributes,
        elem_t * list);

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
 * @param list
 */
void dispatch(CONTENT_t * CONTENT, elem_t **plist)
{
    PARSE_t *PARSE = CONTENT->PARSE;
    LIST_t * LIST = (LIST_t *)PARSE;
    elem_t *list, *elem, *nodes, *edges, *attributes;
    
    assert(plist);
    list = *plist;

    assert(list);
    assert(list->type == (char)LISTELEM);

//E(LIST);
//P(LIST, list);

    nodes = new_list(LIST, 0);
    edges = new_list(LIST, 0);
    attributes = new_list(LIST, 0);

    // expand OBJECT_LIST and ENDPOINTSETS
    dispatch_r(LIST, list, attributes, nodes, edges);

    // if NODE ACT ... for each NODE from nodes, generate new ACT: verb node attributes
    // else if EDGE ACT ... for each NODEREF, generate new ACT: verb node
    //                      for each EDGE, generate new ACT: verb edge attributes

    free_list(LIST, list);  // free old ACT to be replace by these new expanded ACTS
    *plist = list = new_list(LIST, 0);

    switch (CONTENT->subject_type) {
    case NODE:
        elem = nodes->u.l.first;
        while (elem) {
            assemble_act(LIST,elem,attributes,list);
            elem = elem->next;
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
                    assemble_act(LIST,elem,NULL,list);
                    elem = elem->next;
                }

                elem = edges->u.l.first;
                while (elem) {
                    assemble_act(LIST,elem,attributes,list);
                    elem = elem->next;
                }
        }
        break;
    default:
        assert(0);  // shouldn't happen
        break;
    }

    free_list(LIST, attributes);
    free_list(LIST, nodes);
    free_list(LIST, edges);

//E(LIST);
}

/**
 * This function expands OBJECT_LIST of NODES or EDGES, and then expands ENPOINTSETS in EDGES
 *
 * @param LIST context
 * @param list   -- object-list
 * @param attributes
 * @param nodes
 * @param edges
 */
static void
dispatch_r(LIST_t * LIST, elem_t * list, elem_t * attributes,
        elem_t * nodes, elem_t * edges)
{
    elem_t *elem, *new, *object;

    assert(list->type == (char)LISTELEM);
//E(LIST);
//P(LIST, list);

    elem = list->u.l.first;
    while (elem) {
        switch ((state_t) elem->state) {
        case ACT:
        case OBJECT:
            dispatch_r(LIST, elem, attributes, nodes, edges);
            break;
        case ATTRIBUTES:
            new = ref_list(LIST, elem);
            append_list(attributes, new);
            break;
        case SUBJECT:
            object = elem->u.l.first;
            switch ((state_t)object->state) {
            case OBJECT:
                dispatch_r(LIST, object, attributes, nodes, edges);
                break;
            case OBJECT_LIST:
                object = object->u.l.first;
                assert((state_t)object->state == OBJECT);
                while(object) {
                    dispatch_r(LIST, object, attributes, nodes, edges);
                    object = object->next;
                }
                break;
            default:
                fprintf(stderr, "unexpected object->state: %d\n", object->state);
                assert(0); //should never get here
                break;
            }
            break;
        case NODE:
            new = ref_list(LIST, elem);
            append_list(nodes, new);
            break;
        case EDGE:
            expand(LIST, elem, nodes, edges);
            break;
        default:
            fprintf(stderr, "unexpected elem->state: %d\n", elem->state);
            assert(0);
            break;
        }
        elem = elem->next;
    }
//E(LIST);
}

/**
 * This function assembles ACTS with simplified SUBJECT and no CONTAINER
 *
 * @param LIST context
 * @param elem   -- node or edge object
 * @param attributes
 * @param list - output ACT
 */
static void
assemble_act(LIST_t * LIST, elem_t *elem, elem_t *attributes,
        elem_t *list)
{
    TOKEN_t * IN = (TOKEN_t *)LIST;
    elem_t *new;
    elem_t *act;

    act = new_list(LIST, ACT);

    // verb
    switch(IN->verb) {
    case QRY:
    case TLD:
        new = new_list(LIST, IN->verb);
        append_list(act, new);
        break;
    default:
        break;
    }

    // subject
    new = ref_list(LIST, elem);
    append_list(act, new);

    // attributes
    if (attributes && attributes->u.l.first) {
        new = ref_list(LIST, attributes->u.l.first);
        append_list(act, new);
    }

    // no container ever because contents are handled in a parse_nest_r() recursion.

    append_list(list, act);

    free_list(LIST, act);
}
